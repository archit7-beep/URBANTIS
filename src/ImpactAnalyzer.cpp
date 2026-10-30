#include "ImpactAnalyzer.hpp"
#include <algorithm>
#include <sstream>
#include <queue>

namespace urbantis {

ImpactAnalyzer::ImpactAnalyzer(CityGraph& graph) : graph_(graph) {
    // Automatically build dependency maps from graph's cyber devices
    for (const auto& kv : graph_.getAllCyberDevices()) {
        const auto& dev = kv.second;
        if (dev.boundIntersectionId != 0) {
            deviceIntersectionMap_[dev.id] = dev.boundIntersectionId;
        }
        if (!dev.parentGatewayId.empty()) {
            deviceChildrenMap_[dev.parentGatewayId].push_back(dev.id);
            deviceParentMap_[dev.id] = dev.parentGatewayId;
        }
    }
}

void ImpactAnalyzer::registerDependency(const std::string& parentDeviceId, const std::string& dependentDeviceId) {
    deviceChildrenMap_[parentDeviceId].push_back(dependentDeviceId);
    deviceParentMap_[dependentDeviceId] = parentDeviceId;
}

void ImpactAnalyzer::mapDeviceToIntersection(const std::string& deviceId, uint32_t intersectionId) {
    deviceIntersectionMap_[deviceId] = intersectionId;
}

ImpactAssessment ImpactAnalyzer::evaluateDeviceCompromise(const std::string& compromisedDeviceId) {
    ImpactAssessment assessment;
    assessment.incidentDeviceId = compromisedDeviceId;

    std::unordered_set<std::string> affectedDevices;
    std::queue<std::string> q;

    q.push(compromisedDeviceId);
    affectedDevices.insert(compromisedDeviceId);

    // BFS downward in cyber dependency tree
    while (!q.empty()) {
        std::string curr = q.front();
        q.pop();

        auto it = deviceChildrenMap_.find(curr);
        if (it != deviceChildrenMap_.end()) {
            for (const auto& child : it->second) {
                if (affectedDevices.find(child) == affectedDevices.end()) {
                    affectedDevices.insert(child);
                    q.push(child);
                }
            }
        }
    }

    // Map affected cyber devices to physical intersections
    std::unordered_set<uint32_t> directIntersections;
    for (const auto& devId : affectedDevices) {
        auto it = deviceIntersectionMap_.find(devId);
        if (it != deviceIntersectionMap_.end()) {
            directIntersections.insert(it->second);
        }
    }

    for (uint32_t id : directIntersections) {
        assessment.directlyAffectedIntersections.push_back(id);
    }

    // Evaluate physical secondary cascading effect (adjacent intersections)
    std::unordered_set<uint32_t> secondaryIntersections;
    for (uint32_t directId : directIntersections) {
        const auto& outgoing = graph_.getOutgoingRoads(directId);
        for (const auto& road : outgoing) {
            if (directIntersections.find(road.toIntersection) == directIntersections.end()) {
                secondaryIntersections.insert(road.toIntersection);
            }
        }
    }

    for (uint32_t secId : secondaryIntersections) {
        assessment.secondaryCascadingIntersections.push_back(secId);
    }

    // Check Articulation Points
    std::vector<uint32_t> artPoints = graph_.findArticulationPoints();
    for (uint32_t directId : directIntersections) {
        if (std::find(artPoints.begin(), artPoints.end(), directId) != artPoints.end()) {
            assessment.articulationPointCompromised = true;
            break;
        }
    }

    // Check hospital vicinity impairment (Intersections 1, 5, 25 are hospital/critical hubs)
    for (uint32_t directId : directIntersections) {
        if (directId == 1 || directId == 5 || directId == 25) {
            assessment.hospitalAccessImpaired = true;
            break;
        }
    }

    // Estimate systemic delay increase
    double baseIncrease = assessment.directlyAffectedIntersections.size() * 6.5;
    if (assessment.articulationPointCompromised) baseIncrease += 25.0;
    if (assessment.hospitalAccessImpaired) baseIncrease += 20.0;
    assessment.estimatedDelayIncreasePercent = std::min(100.0, baseIncrease);

    // Compute composite disruption severity score (0 to 100)
    double score = (assessment.directlyAffectedIntersections.size() * 5.0) +
                   (assessment.secondaryCascadingIntersections.size() * 2.0);
    if (assessment.articulationPointCompromised) score += 30.0;
    if (assessment.hospitalAccessImpaired) score += 25.0;
    assessment.overallDisruptionSeverityScore = std::min(100.0, score);

    std::ostringstream oss;
    oss << "Compromise of " << compromisedDeviceId << " cascades to " 
        << assessment.directlyAffectedIntersections.size() << " primary intersections and "
        << assessment.secondaryCascadingIntersections.size() << " secondary buffer nodes. "
        << (assessment.articulationPointCompromised ? "CRITICAL: Articulation bottleneck compromised! " : "")
        << (assessment.hospitalAccessImpaired ? "WARNING: Emergency hospital corridor accessibility impaired!" : "Corridor connectivity maintained.");
    assessment.impactSummary = oss.str();

    return assessment;
}

ImpactAssessment ImpactAnalyzer::evaluateAlertImpact(const CyberAlert& alert) {
    return evaluateDeviceCompromise(alert.targetDeviceId.empty() ? alert.sourceDeviceId : alert.targetDeviceId);
}

std::vector<std::string> ImpactAnalyzer::identifyCriticalCyberGateways() const {
    std::vector<std::pair<size_t, std::string>> ranked;
    for (const auto& kv : deviceChildrenMap_) {
        ranked.emplace_back(kv.second.size(), kv.first);
    }
    std::sort(ranked.rbegin(), ranked.rend());

    std::vector<std::string> critical;
    for (const auto& item : ranked) {
        critical.push_back(item.second);
    }
    return critical;
}

} // namespace urbantis
