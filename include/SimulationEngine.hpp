#pragma once

#include "Types.hpp"
#include "CityGraph.hpp"
#include "TrafficEngine.hpp"
#include "ServiceAssignment.hpp"
#include "EmergencyManager.hpp"
#include "ThreatDetector.hpp"
#include "ImpactAnalyzer.hpp"
#include "ResponseOptimizer.hpp"
#include <memory>
#include <string>

namespace urbantis {

struct BenchmarkSuiteResults {
    // 1. Dijkstra vs A*
    double dijkstraAvgMicros{0.0};
    double aStarAvgMicros{0.0};
    double aStarSpeedupMultiplier{0.0};
    double nodeExplorationReductionPercent{0.0};

    // 2. Greedy vs Optimal Bipartite Delivery Matching
    double greedyFeasibilityPercent{0.0};
    double optimalFeasibilityPercent{0.0};
    double travelTimeReductionPercent{0.0};

    // 3. Cyber Threat Detection Throughput
    double threatDetectionThroughputEventsPerSec{0.0};
    size_t alertsGenerated{0};

    // 4. Incident Response Optimization
    double passiveMonitorScore{0.0};
    double aggressiveIsolationScore{0.0};
    double surgicalFailoverScore{0.0};
    double emergencyCorridorScore{0.0};
    std::string recommendedStrategy;
};

class SimulationEngine {
public:
    SimulationEngine();

    // Simulation lifecycle
    void initializeScenario(uint32_t gridRows = 5, uint32_t gridCols = 5);
    void step(double deltaSeconds = 1.0);

    // Scenario Injections
    void triggerDDoSAttackOnGateway(const std::string& gatewayId = "GW-NORTH-01", size_t burstSize = 250);
    void triggerKillchainAttack(const std::string& attackerIp = "192.168.1.100", const std::string& targetDevice = "SIG-CTRL-13");
    void triggerTrafficAccident(uint32_t intersectionId = 14, int severity = 5);
    void submitRandomDeliveryOrder();

    // Run Comprehensive Algorithm Benchmark Suite
    BenchmarkSuiteResults runFullBenchmarkSuite(size_t trials = 100);

    // Subsystem accessors
    CityGraph& getGraph() { return graph_; }
    const CityGraph& getGraph() const { return graph_; }
    TrafficEngine& getTrafficEngine() { return trafficEngine_; }
    const TrafficEngine& getTrafficEngine() const { return trafficEngine_; }
    ServiceAssignment& getServiceAssignment() { return serviceAssignment_; }
    const ServiceAssignment& getServiceAssignment() const { return serviceAssignment_; }
    EmergencyManager& getEmergencyManager() { return emergencyManager_; }
    const EmergencyManager& getEmergencyManager() const { return emergencyManager_; }
    ThreatDetector& getThreatDetector() { return threatDetector_; }
    const ThreatDetector& getThreatDetector() const { return threatDetector_; }
    ImpactAnalyzer& getImpactAnalyzer() { return impactAnalyzer_; }
    const ImpactAnalyzer& getImpactAnalyzer() const { return impactAnalyzer_; }
    ResponseOptimizer& getResponseOptimizer() { return responseOptimizer_; }
    const ResponseOptimizer& getResponseOptimizer() const { return responseOptimizer_; }

    uint64_t getCurrentTick() const { return currentTick_; }
    double getSimulationTimeSec() const { return simulationTimeSec_; }

private:
    uint64_t currentTick_{0};
    double simulationTimeSec_{0.0};

    CityGraph graph_;
    TrafficEngine trafficEngine_;
    ServiceAssignment serviceAssignment_;
    EmergencyManager emergencyManager_;
    ThreatDetector threatDetector_;
    ImpactAnalyzer impactAnalyzer_;
    ResponseOptimizer responseOptimizer_;

    uint32_t incidentCounter_{1};
    uint32_t requestCounter_{1};

    void seedInitialCityResources();
};

} // namespace urbantis
