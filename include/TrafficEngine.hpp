#pragma once

#include "CityGraph.hpp"
#include "Types.hpp"
#include <vector>
#include <queue>
#include <unordered_map>
#include <chrono>

namespace urbantis {

struct RouteResult {
    bool found{false};
    std::vector<uint32_t> path;             // Intersection IDs
    std::vector<uint32_t> roadSegmentIds;    // Road IDs traversed
    double totalDistanceMeters{0.0};
    double totalTravelTimeSec{0.0};
    size_t nodesExplored{0};
    double executionTimeMicroseconds{0.0};
};

struct SignalTimingProfile {
    uint32_t intersectionId{0};
    double northSouthGreenSec{30.0};
    double eastWestGreenSec{30.0};
    double totalCycleSec{60.0};
    double saturationFlowRatio{0.0};
};

class TrafficEngine {
public:
    explicit TrafficEngine(CityGraph& graph);

    // Shortest & Fast Path Planning
    RouteResult computeShortestPathDijkstra(uint32_t startId, uint32_t goalId, VehicleType vtype = VehicleType::CAR);
    RouteResult computeShortestPathAStar(uint32_t startId, uint32_t goalId, VehicleType vtype = VehicleType::CAR);

    // K-Alternative Routes (Penalty-based diverse route generation)
    std::vector<RouteResult> computeKAlternativeRoutes(uint32_t startId, uint32_t goalId, size_t k = 3, VehicleType vtype = VehicleType::CAR);

    // Dynamic Congestion & Signal Control
    void simulateTrafficStep(double deltaSeconds);
    void updateAdaptiveSignals();
    SignalTimingProfile calculateWebsterCycle(uint32_t intersectionId);

    // Preemption for emergency corridors
    void establishEmergencyGreenCorridor(const std::vector<uint32_t>& path);
    void clearEmergencyPreemption(const std::vector<uint32_t>& path);

    // Comparative Benchmarking
    struct RoutingBenchmarkComparison {
        RouteResult dijkstraResult;
        RouteResult aStarResult;
        double speedupRatio;
        size_t nodeExplorationReduction;
    };
    RoutingBenchmarkComparison runRoutingBenchmark(uint32_t startId, uint32_t goalId, VehicleType vtype = VehicleType::CAR);

private:
    CityGraph& graph_;

    // A* Heuristic: admissible travel time lower bound
    double calculateHeuristicTime(uint32_t currentId, uint32_t goalId, VehicleType vtype) const;
};

} // namespace urbantis
