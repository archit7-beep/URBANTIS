#include "ServiceAssignment.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace urbantis {

ServiceAssignment::ServiceAssignment(CityGraph& graph, TrafficEngine& trafficEngine)
    : graph_(graph), trafficEngine_(trafficEngine) {}

void ServiceAssignment::registerRider(const CourierRider& rider) {
    riders_.push_back(rider);
}

void ServiceAssignment::submitServiceRequest(const ServiceRequest& request) {
    pendingRequests_.push_back(request);
}

AssignmentReport ServiceAssignment::executeGreedyAssignment(uint64_t currentSimulationTick) {
    AssignmentReport report;
    if (pendingRequests_.empty() || riders_.empty()) {
        report.requestsPending = pendingRequests_.size();
        return report;
    }

    // Sort pending requests by urgency: Priority (high first) and Deadline (earliest first)
    std::sort(pendingRequests_.begin(), pendingRequests_.end(),
        [](const ServiceRequest& a, const ServiceRequest& b) {
            if (a.priority != b.priority) {
                return a.priority > b.priority;
            }
            return a.deadlineTime < b.deadlineTime;
        });

    std::vector<ServiceRequest> remaining;
    size_t feasibleCount = 0;

    for (auto& req : pendingRequests_) {
        int bestRiderIdx = -1;
        double minTotalTime = std::numeric_limits<double>::max();
        RouteResult bestPickupRoute;
        RouteResult bestDeliveryRoute;

        for (size_t i = 0; i < riders_.size(); ++i) {
            auto& rider = riders_[i];
            if (!rider.isAvailable || rider.currentLoad >= rider.maxCapacity) continue;

            // Route 1: Rider -> Pickup
            RouteResult pickupRoute = trafficEngine_.computeShortestPathAStar(
                rider.currentIntersectionId, req.pickupIntersectionId, rider.vehicleType);
            if (!pickupRoute.found) continue;

            // Route 2: Pickup -> Dropoff
            RouteResult deliveryRoute = trafficEngine_.computeShortestPathAStar(
                req.pickupIntersectionId, req.dropoffIntersectionId, rider.vehicleType);
            if (!deliveryRoute.found) continue;

            double totalTripTime = pickupRoute.totalTravelTimeSec + deliveryRoute.totalTravelTimeSec;
            if (totalTripTime < minTotalTime) {
                minTotalTime = totalTripTime;
                bestRiderIdx = static_cast<int>(i);
                bestPickupRoute = pickupRoute;
                bestDeliveryRoute = deliveryRoute;
            }
        }

        if (bestRiderIdx != -1) {
            auto& rider = riders_[bestRiderIdx];
            rider.currentLoad++;
            req.assignedRiderId = rider.id;
            rider.assignedJobs.push_back(req);

            // Construct full planned path
            for (uint32_t node : bestPickupRoute.path) {
                rider.currentPlannedRoute.push_back(node);
            }
            for (size_t p = 1; p < bestDeliveryRoute.path.size(); ++p) {
                rider.currentPlannedRoute.push_back(bestDeliveryRoute.path[p]);
            }

            report.requestsMatched++;
            report.totalEstimatedTravelTimeSec += minTotalTime;

            bool isFeasible = (currentSimulationTick + minTotalTime <= req.deadlineTime);
            if (isFeasible) {
                feasibleCount++;
            }
        } else {
            remaining.push_back(req);
        }
    }

    report.requestsPending = remaining.size();
    if (report.requestsMatched > 0) {
        report.deadlineFeasibilityRate = (static_cast<double>(feasibleCount) / report.requestsMatched) * 100.0;
    }
    pendingRequests_ = std::move(remaining);
    return report;
}

AssignmentReport ServiceAssignment::executeOptimalBipartiteAssignment(uint64_t currentSimulationTick) {
    AssignmentReport report;
    if (pendingRequests_.empty() || riders_.empty()) {
        report.requestsPending = pendingRequests_.size();
        return report;
    }

    // Filter available riders
    std::vector<size_t> availableRiderIndices;
    for (size_t i = 0; i < riders_.size(); ++i) {
        if (riders_[i].isAvailable && riders_[i].currentLoad < riders_[i].maxCapacity) {
            availableRiderIndices.push_back(i);
        }
    }
    if (availableRiderIndices.empty()) {
        report.requestsPending = pendingRequests_.size();
        return report;
    }

    size_t numRiders = availableRiderIndices.size();
    size_t numRequests = std::min(pendingRequests_.size(), numRiders * 2);

    // Build cost matrix: Cost(r, q) = TravelTime + DeadlinePenalty - PriorityBonus
    std::vector<std::vector<double>> costMatrix(numRiders, std::vector<double>(numRequests, 1e9));

    for (size_t r = 0; r < numRiders; ++r) {
        const auto& rider = riders_[availableRiderIndices[r]];
        for (size_t q = 0; q < numRequests; ++q) {
            const auto& req = pendingRequests_[q];

            RouteResult r1 = trafficEngine_.computeShortestPathAStar(
                rider.currentIntersectionId, req.pickupIntersectionId, rider.vehicleType);
            RouteResult r2 = trafficEngine_.computeShortestPathAStar(
                req.pickupIntersectionId, req.dropoffIntersectionId, rider.vehicleType);

            if (r1.found && r2.found) {
                double time = r1.totalTravelTimeSec + r2.totalTravelTimeSec;
                double priorityBonus = req.priority * 150.0;
                double deadlinePenalty = 0.0;
                if (currentSimulationTick + time > req.deadlineTime) {
                    deadlinePenalty = 500.0 * (1.0 + (currentSimulationTick + time - req.deadlineTime) / 60.0);
                }
                costMatrix[r][q] = time + deadlinePenalty - priorityBonus;
            }
        }
    }

    // Solve Minimum Cost Matching (Greedy global search over cost matrix with bipartite constraint)
    std::vector<bool> riderMatched(numRiders, false);
    std::vector<bool> requestMatched(numRequests, false);
    size_t feasibleCount = 0;

    for (size_t iter = 0; iter < std::min(numRiders, numRequests); ++iter) {
        double minCost = 1e8;
        int bestR = -1;
        int bestQ = -1;

        for (size_t r = 0; r < numRiders; ++r) {
            if (riderMatched[r]) continue;
            for (size_t q = 0; q < numRequests; ++q) {
                if (requestMatched[q]) continue;
                if (costMatrix[r][q] < minCost) {
                    minCost = costMatrix[r][q];
                    bestR = static_cast<int>(r);
                    bestQ = static_cast<int>(q);
                }
            }
        }

        if (bestR != -1 && bestQ != -1) {
            riderMatched[bestR] = true;
            requestMatched[bestQ] = true;

            auto& rider = riders_[availableRiderIndices[bestR]];
            auto& req = pendingRequests_[bestQ];

            rider.currentLoad++;
            req.assignedRiderId = rider.id;
            rider.assignedJobs.push_back(req);

            RouteResult r1 = trafficEngine_.computeShortestPathAStar(
                rider.currentIntersectionId, req.pickupIntersectionId, rider.vehicleType);
            RouteResult r2 = trafficEngine_.computeShortestPathAStar(
                req.pickupIntersectionId, req.dropoffIntersectionId, rider.vehicleType);

            for (uint32_t node : r1.path) rider.currentPlannedRoute.push_back(node);
            for (size_t p = 1; p < r2.path.size(); ++p) rider.currentPlannedRoute.push_back(r2.path[p]);

            double tripTime = r1.totalTravelTimeSec + r2.totalTravelTimeSec;
            report.requestsMatched++;
            report.totalEstimatedTravelTimeSec += tripTime;

            if (currentSimulationTick + tripTime <= req.deadlineTime) {
                feasibleCount++;
            }
        }
    }

    // Update pending list
    std::vector<ServiceRequest> remaining;
    for (size_t q = 0; q < pendingRequests_.size(); ++q) {
        if (q >= numRequests || !requestMatched[q]) {
            remaining.push_back(pendingRequests_[q]);
        }
    }
    pendingRequests_ = std::move(remaining);
    report.requestsPending = pendingRequests_.size();
    if (report.requestsMatched > 0) {
        report.deadlineFeasibilityRate = (static_cast<double>(feasibleCount) / report.requestsMatched) * 100.0;
    }

    return report;
}

double ServiceAssignment::calculateTourDistance(const std::vector<uint32_t>& tour) {
    double totalDist = 0.0;
    for (size_t i = 0; i + 1 < tour.size(); ++i) {
        if (graph_.hasIntersection(tour[i]) && graph_.hasIntersection(tour[i + 1])) {
            totalDist += graph_.getIntersection(tour[i]).location.distanceTo(
                graph_.getIntersection(tour[i + 1]).location);
        }
    }
    return totalDist;
}

std::vector<uint32_t> ServiceAssignment::optimizeMultiStopRoute(int riderId, const std::vector<uint32_t>& stopIntersections) {
    (void)riderId;
    if (stopIntersections.size() <= 2) return stopIntersections;

    // Start with initial tour
    std::vector<uint32_t> tour = stopIntersections;
    bool improved = true;
    double bestDist = calculateTourDistance(tour);

    // 2-Opt local search
    while (improved) {
        improved = false;
        for (size_t i = 1; i < tour.size() - 2; ++i) {
            for (size_t k = i + 1; k < tour.size() - 1; ++k) {
                // Try 2-opt swap
                std::reverse(tour.begin() + i, tour.begin() + k + 1);
                double newDist = calculateTourDistance(tour);
                if (newDist < bestDist - 1e-4) {
                    bestDist = newDist;
                    improved = true;
                } else {
                    // Revert
                    std::reverse(tour.begin() + i, tour.begin() + k + 1);
                }
            }
        }
    }
    return tour;
}

void ServiceAssignment::updateRiderPositions(double deltaSeconds, uint64_t currentTick) {
    for (auto& rider : riders_) {
        if (rider.currentPlannedRoute.empty()) {
            rider.isAvailable = true;
            continue;
        }

        // Simulate rider movement along path
        rider.totalServiceTimeSec += deltaSeconds;
        // Rider arrives at next intersection
        uint32_t nextNode = rider.currentPlannedRoute.front();
        rider.currentPlannedRoute.erase(rider.currentPlannedRoute.begin());
        rider.currentIntersectionId = nextNode;

        // Check if any job completed at this node
        auto it = rider.assignedJobs.begin();
        while (it != rider.assignedJobs.end()) {
            if (it->dropoffIntersectionId == nextNode) {
                it->fulfilled = true;
                it->completedTime = currentTick;
                it->late = (currentTick > it->deadlineTime);

                rider.totalDeliveriesCompleted++;
                if (!it->late) {
                    rider.onTimeDeliveries++;
                }
                completedRequests_.push_back(*it);
                it = rider.assignedJobs.erase(it);
                rider.currentLoad = std::max(0, rider.currentLoad - 1);
            } else {
                ++it;
            }
        }

        if (rider.assignedJobs.empty()) {
            rider.isAvailable = true;
        }
    }
}

double ServiceAssignment::getOverallOnTimeDeliveryRate() const {
    if (completedRequests_.empty()) return 100.0;
    size_t onTime = 0;
    for (const auto& req : completedRequests_) {
        if (!req.late) onTime++;
    }
    return (static_cast<double>(onTime) / completedRequests_.size()) * 100.0;
}

} // namespace urbantis
