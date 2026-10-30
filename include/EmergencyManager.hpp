#pragma once

#include "Types.hpp"
#include "TrafficEngine.hpp"
#include <vector>
#include <unordered_map>
#include <memory>

namespace urbantis {

struct DispatchReport {
    bool successful{false};
    uint32_t incidentId{0};
    int ambulanceId{-1};
    uint32_t selectedHospitalId{0};
    double estimatedResponseTimeSec{0.0};
    double estimatedHospitalTransitSec{0.0};
    std::string dispatchLog;
};

class EmergencyManager {
public:
    explicit EmergencyManager(CityGraph& graph, TrafficEngine& trafficEngine);

    // Resource Registration
    void registerHospital(const Hospital& hospital);
    void registerAmbulance(const Ambulance& ambulance);

    const std::vector<Hospital>& getAllHospitals() const { return hospitals_; }
    std::vector<Hospital>& getAllHospitals() { return hospitals_; }
    const std::vector<Ambulance>& getAllAmbulances() const { return ambulances_; }
    std::vector<Ambulance>& getAllAmbulances() { return ambulances_; }

    // Emergency Incidents
    void reportIncident(const EmergencyIncident& incident);
    const std::vector<EmergencyIncident>& getActiveIncidents() const { return activeIncidents_; }
    const std::vector<EmergencyIncident>& getResolvedIncidents() const { return resolvedIncidents_; }

    // Multi-Criteria Hospital Selection: Balances travel time, trauma level, and triage queue
    uint32_t selectOptimalHospital(uint32_t sceneIntersectionId, int incidentSeverity);

    // Emergency Dispatch & Corridor Preemption
    DispatchReport dispatchEmergencyResponse(uint32_t incidentId);

    // Dynamic Reroute: If active route encounters cyber-manipulated or blocked road
    bool rerouteActiveAmbulance(int ambulanceId);

    // Simulation Update Tick
    void updateAmbulances(double deltaSeconds, uint64_t currentTick);

    // Statistics & Evaluation
    double getAverageResponseTimeSec() const;
    size_t getSuccessfulDispatches() const { return resolvedIncidents_.size(); }

private:
    CityGraph& graph_;
    TrafficEngine& trafficEngine_;
    std::vector<Hospital> hospitals_;
    std::vector<Ambulance> ambulances_;
    std::vector<EmergencyIncident> activeIncidents_;
    std::vector<EmergencyIncident> resolvedIncidents_;
    std::vector<double> recordedResponseTimes_;
};

} // namespace urbantis
