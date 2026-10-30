#pragma once

#include "Types.hpp"
#include "TrafficEngine.hpp"
#include <vector>
#include <queue>
#include <memory>
#include <algorithm>

namespace urbantis {

struct CourierRider {
    int id{0};
    std::string name;
    uint32_t currentIntersectionId{0};
    VehicleType vehicleType{VehicleType::BIKE};
    bool isAvailable{true};
    int maxCapacity{3};
    int currentLoad{0};
    std::vector<uint32_t> currentPlannedRoute;
    std::vector<ServiceRequest> assignedJobs;
    uint32_t totalDeliveriesCompleted{0};
    uint32_t onTimeDeliveries{0};
    double totalServiceTimeSec{0.0};
};

struct AssignmentReport {
    size_t requestsMatched{0};
    size_t requestsPending{0};
    double totalEstimatedTravelTimeSec{0.0};
    double deadlineFeasibilityRate{0.0}; // Percentage 0 - 100%
};

class ServiceAssignment {
public:
    explicit ServiceAssignment(CityGraph& graph, TrafficEngine& trafficEngine);

    // Rider Management
    void registerRider(const CourierRider& rider);
    std::vector<CourierRider>& getAllRiders() { return riders_; }
    const std::vector<CourierRider>& getAllRiders() const { return riders_; }

    // Request Management
    void submitServiceRequest(const ServiceRequest& request);
    const std::vector<ServiceRequest>& getPendingRequests() const { return pendingRequests_; }
    const std::vector<ServiceRequest>& getCompletedRequests() const { return completedRequests_; }

    // Matching Algorithms:
    // 1. Fast Greedy Earliest-Deadline Matching
    AssignmentReport executeGreedyAssignment(uint64_t currentSimulationTick);

    // 2. Cost-Optimized Bipartite Matching (Weighted Bipartite Matching with deadline penalties)
    AssignmentReport executeOptimalBipartiteAssignment(uint64_t currentSimulationTick);

    // Multi-stop Courier Routing: 2-Opt Local Search for optimal stop sequencing
    std::vector<uint32_t> optimizeMultiStopRoute(int riderId, const std::vector<uint32_t>& stopIntersections);

    // Simulation Tick Update
    void updateRiderPositions(double deltaSeconds, uint64_t currentTick);

    // Performance Metrics
    double getOverallOnTimeDeliveryRate() const;
    size_t getTotalCompletedDeliveries() const { return completedRequests_.size(); }

private:
    CityGraph& graph_;
    TrafficEngine& trafficEngine_;
    std::vector<CourierRider> riders_;
    std::vector<ServiceRequest> pendingRequests_;
    std::vector<ServiceRequest> completedRequests_;

    // 2-Opt TSP helper
    double calculateTourDistance(const std::vector<uint32_t>& tour);
};

} // namespace urbantis
