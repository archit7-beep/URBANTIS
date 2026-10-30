#include "EmergencyManager.hpp"
#include <algorithm>
#include <limits>
#include <sstream>

namespace urbantis {

EmergencyManager::EmergencyManager(CityGraph& graph, TrafficEngine& trafficEngine)
    : graph_(graph), trafficEngine_(trafficEngine) {}

void EmergencyManager::registerHospital(const Hospital& hospital) {
    hospitals_.push_back(hospital);
}

void EmergencyManager::registerAmbulance(const Ambulance& ambulance) {
    ambulances_.push_back(ambulance);
}

void EmergencyManager::reportIncident(const EmergencyIncident& incident) {
    activeIncidents_.push_back(incident);
}

uint32_t EmergencyManager::selectOptimalHospital(uint32_t sceneIntersectionId, int incidentSeverity) {
    if (hospitals_.empty()) return 0;

    uint32_t bestHospitalId = hospitals_[0].id;
    double minCompositeScore = std::numeric_limits<double>::max();

    for (const auto& hosp : hospitals_) {
        if (hosp.availableBeds == 0) continue;

        // 1. Travel time
        RouteResult rr = trafficEngine_.computeShortestPathAStar(
            sceneIntersectionId, hosp.intersectionId, VehicleType::AMBULANCE);
        double travelTime = rr.found ? rr.totalTravelTimeSec : 1e6;

        // 2. Trauma level match penalty (severe incidents need Level 1 trauma care)
        double traumaPenalty = 0.0;
        if (incidentSeverity >= 4 && hosp.traumaLevel > 1) {
            traumaPenalty = 300.0; // 5 minute penalty for inadequate trauma center
        }

        // 3. Triage queue delay
        double queueDelay = hosp.triageQueueDelaySec;

        // 4. Bed availability bonus
        double bedBonus = std::min(50.0, static_cast<double>(hosp.availableBeds) * 1.5);

        double compositeScore = travelTime + queueDelay + traumaPenalty - bedBonus;

        if (compositeScore < minCompositeScore) {
            minCompositeScore = compositeScore;
            bestHospitalId = hosp.id;
        }
    }
    return bestHospitalId;
}

DispatchReport EmergencyManager::dispatchEmergencyResponse(uint32_t incidentId) {
    DispatchReport report;
    report.incidentId = incidentId;

    auto incIt = std::find_if(activeIncidents_.begin(), activeIncidents_.end(),
        [incidentId](const EmergencyIncident& inc) { return inc.id == incidentId; });

    if (incIt == activeIncidents_.end()) {
        report.dispatchLog = "Incident not found.";
        return report;
    }

    // 1. Select optimal hospital
    uint32_t hospitalId = selectOptimalHospital(incIt->intersectionId, incIt->severity);
    incIt->assignedHospitalId = hospitalId;
    report.selectedHospitalId = hospitalId;

    auto hospIt = std::find_if(hospitals_.begin(), hospitals_.end(),
        [hospitalId](const Hospital& h) { return h.id == hospitalId; });
    uint32_t hospitalIntersection = (hospIt != hospitals_.end()) ? hospIt->intersectionId : incIt->intersectionId;

    // 2. Find closest available ambulance
    int bestAmbIdx = -1;
    double minEtaToScene = std::numeric_limits<double>::max();
    RouteResult bestRouteToScene;

    for (size_t i = 0; i < ambulances_.size(); ++i) {
        if (!ambulances_[i].isAvailable) continue;

        RouteResult r = trafficEngine_.computeShortestPathAStar(
            ambulances_[i].currentIntersectionId, incIt->intersectionId, VehicleType::AMBULANCE);
        if (r.found && r.totalTravelTimeSec < minEtaToScene) {
            minEtaToScene = r.totalTravelTimeSec;
            bestAmbIdx = static_cast<int>(i);
            bestRouteToScene = r;
        }
    }

    if (bestAmbIdx == -1) {
        report.dispatchLog = "No available ambulances currently in service.";
        return report;
    }

    auto& amb = ambulances_[bestAmbIdx];
    amb.isAvailable = false;
    amb.currentEmergencyId = incidentId;
    incIt->assignedAmbulanceId = amb.id;
    report.ambulanceId = amb.id;
    report.estimatedResponseTimeSec = minEtaToScene;

    // Route from Scene to Hospital
    RouteResult sceneToHospital = trafficEngine_.computeShortestPathAStar(
        incIt->intersectionId, hospitalIntersection, VehicleType::AMBULANCE);
    report.estimatedHospitalTransitSec = sceneToHospital.totalTravelTimeSec;

    // Preempt green traffic lights along ambulance route
    trafficEngine_.establishEmergencyGreenCorridor(bestRouteToScene.path);

    // Build full path
    amb.currentPath = bestRouteToScene.path;
    for (size_t k = 1; k < sceneToHospital.path.size(); ++k) {
        amb.currentPath.push_back(sceneToHospital.path[k]);
    }
    amb.currentPathIndex = 0;

    incIt->travelTimeToSceneSec = minEtaToScene;
    incIt->travelTimeToHospitalSec = sceneToHospital.totalTravelTimeSec;

    report.successful = true;
    std::ostringstream oss;
    oss << "Ambulance " << amb.name << " dispatched to Scene (" << incIt->intersectionId 
        << "), ETA: " << static_cast<int>(minEtaToScene) << "s. Transferring to Hospital: "
        << ((hospIt != hospitals_.end()) ? hospIt->name : "General Hospital");
    report.dispatchLog = oss.str();

    return report;
}

bool EmergencyManager::rerouteActiveAmbulance(int ambulanceId) {
    auto ambIt = std::find_if(ambulances_.begin(), ambulances_.end(),
        [ambulanceId](const Ambulance& a) { return a.id == ambulanceId; });

    if (ambIt == ambulances_.end() || ambIt->isAvailable || ambIt->currentPath.empty()) {
        return false;
    }

    uint32_t currentPos = ambIt->currentIntersectionId;
    uint32_t destination = ambIt->currentPath.back();

    RouteResult newRoute = trafficEngine_.computeShortestPathAStar(
        currentPos, destination, VehicleType::AMBULANCE);

    if (newRoute.found) {
        // Clear prior corridor preemption
        trafficEngine_.clearEmergencyPreemption(ambIt->currentPath);

        // Apply new route and preempt new corridor
        ambIt->currentPath = newRoute.path;
        ambIt->currentPathIndex = 0;
        trafficEngine_.establishEmergencyGreenCorridor(newRoute.path);
        return true;
    }
    return false;
}

void EmergencyManager::updateAmbulances(double deltaSeconds, uint64_t currentTick) {
    (void)deltaSeconds;
    (void)currentTick;
    for (auto& amb : ambulances_) {
        if (amb.isAvailable || amb.currentPath.empty()) continue;

        // Progress ambulance along route
        if (amb.currentPathIndex + 1 < amb.currentPath.size()) {
            amb.currentPathIndex++;
            amb.currentIntersectionId = amb.currentPath[amb.currentPathIndex];
        } else {
            // Reached destination (hospital)
            trafficEngine_.clearEmergencyPreemption(amb.currentPath);
            amb.isAvailable = true;
            amb.currentPath.clear();
            amb.currentPathIndex = 0;

            // Resolve incident
            int incId = amb.currentEmergencyId;
            auto it = std::find_if(activeIncidents_.begin(), activeIncidents_.end(),
                [incId](const EmergencyIncident& inc) { return static_cast<int>(inc.id) == incId; });

            if (it != activeIncidents_.end()) {
                it->resolved = true;
                double totalTime = it->travelTimeToSceneSec + it->travelTimeToHospitalSec;
                recordedResponseTimes_.push_back(totalTime);
                resolvedIncidents_.push_back(*it);
                activeIncidents_.erase(it);
            }
            amb.currentEmergencyId = -1;
        }
    }
}

double EmergencyManager::getAverageResponseTimeSec() const {
    if (recordedResponseTimes_.empty()) return 0.0;
    double sum = 0.0;
    for (double t : recordedResponseTimes_) sum += t;
    return sum / recordedResponseTimes_.size();
}

} // namespace urbantis
