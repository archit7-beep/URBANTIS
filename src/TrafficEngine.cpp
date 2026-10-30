#include "TrafficEngine.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <tuple>

namespace urbantis {

TrafficEngine::TrafficEngine(CityGraph& graph) : graph_(graph) {}

double TrafficEngine::calculateHeuristicTime(uint32_t currentId, uint32_t goalId, VehicleType vtype) const {
    if (!graph_.hasIntersection(currentId) || !graph_.hasIntersection(goalId)) {
        return 0.0;
    }
    const auto& curr = graph_.getIntersection(currentId);
    const auto& goal = graph_.getIntersection(goalId);
    double dist = curr.location.distanceTo(goal.location);

    // Max conceivable speed for admissible heuristic (never overestimates)
    double maxSpeed = 25.0; // 90 km/h (Express Highway max)
    if (vtype == VehicleType::AMBULANCE) {
        maxSpeed = 30.0;
    }
    return dist / maxSpeed;
}

RouteResult TrafficEngine::computeShortestPathDijkstra(uint32_t startId, uint32_t goalId, VehicleType vtype) {
    auto startTime = std::chrono::high_resolution_clock::now();
    RouteResult res;

    if (!graph_.hasIntersection(startId) || !graph_.hasIntersection(goalId)) {
        return res;
    }
    if (graph_.getIntersection(startId).isBlocked || graph_.getIntersection(goalId).isBlocked) {
        return res;
    }

    // Min-heap: pair<cost, intersectionId>
    typedef std::pair<double, uint32_t> PQElement;
    std::priority_queue<PQElement, std::vector<PQElement>, std::greater<PQElement>> pq;

    std::unordered_map<uint32_t, double> dist;
    std::unordered_map<uint32_t, uint32_t> parentNode;
    std::unordered_map<uint32_t, RoadSegment> parentRoad;

    dist[startId] = 0.0;
    pq.emplace(0.0, startId);

    size_t explored = 0;

    while (!pq.empty()) {
        auto top = pq.top();
        pq.pop();
        double currentCost = top.first;
        uint32_t u = top.second;
        explored++;

        if (currentCost > dist[u]) continue;
        if (u == goalId) break;

        const auto& outgoing = graph_.getOutgoingRoads(u);
        for (const auto& road : outgoing) {
            if (road.isClosed) continue;
            uint32_t v = road.toIntersection;
            if (graph_.getIntersection(v).isBlocked) continue;

            double edgeCost = road.getDynamicTravelTime(vtype);
            double newCost = currentCost + edgeCost;

            if (dist.find(v) == dist.end() || newCost < dist[v]) {
                dist[v] = newCost;
                parentNode[v] = u;
                parentRoad[v] = road;
                pq.emplace(newCost, v);
            }
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    res.executionTimeMicroseconds = std::chrono::duration<double, std::micro>(endTime - startTime).count();
    res.nodesExplored = explored;

    if (dist.find(goalId) == dist.end()) {
        res.found = false;
        return res;
    }

    res.found = true;
    res.totalTravelTimeSec = dist[goalId];

    // Reconstruct path
    uint32_t curr = goalId;
    while (curr != startId) {
        res.path.push_back(curr);
        RoadSegment rd = parentRoad[curr];
        res.roadSegmentIds.push_back(rd.id);
        res.totalDistanceMeters += rd.lengthMeters;
        curr = parentNode[curr];
    }
    res.path.push_back(startId);
    std::reverse(res.path.begin(), res.path.end());
    std::reverse(res.roadSegmentIds.begin(), res.roadSegmentIds.end());

    return res;
}

RouteResult TrafficEngine::computeShortestPathAStar(uint32_t startId, uint32_t goalId, VehicleType vtype) {
    auto startTime = std::chrono::high_resolution_clock::now();
    RouteResult res;

    if (!graph_.hasIntersection(startId) || !graph_.hasIntersection(goalId)) {
        return res;
    }
    if (graph_.getIntersection(startId).isBlocked || graph_.getIntersection(goalId).isBlocked) {
        return res;
    }

    // Min-heap tuple: <fScore, gScore, intersectionId>
    typedef std::tuple<double, double, uint32_t> AStarTuple;
    auto cmp = [](const AStarTuple& a, const AStarTuple& b) {
        return std::get<0>(a) > std::get<0>(b);
    };
    std::priority_queue<AStarTuple, std::vector<AStarTuple>, decltype(cmp)> pq(cmp);

    std::unordered_map<uint32_t, double> gScore;
    std::unordered_map<uint32_t, uint32_t> parentNode;
    std::unordered_map<uint32_t, RoadSegment> parentRoad;

    gScore[startId] = 0.0;
    double initialH = calculateHeuristicTime(startId, goalId, vtype);
    pq.emplace(initialH, 0.0, startId);

    size_t explored = 0;

    while (!pq.empty()) {
        auto top = pq.top();
        pq.pop();
        double currentG = std::get<1>(top);
        uint32_t u = std::get<2>(top);
        explored++;

        if (currentG > gScore[u]) continue;
        if (u == goalId) break;

        const auto& outgoing = graph_.getOutgoingRoads(u);
        for (const auto& road : outgoing) {
            if (road.isClosed) continue;
            uint32_t v = road.toIntersection;
            if (graph_.getIntersection(v).isBlocked) continue;

            double edgeCost = road.getDynamicTravelTime(vtype);
            double tentativeG = currentG + edgeCost;

            if (gScore.find(v) == gScore.end() || tentativeG < gScore[v]) {
                gScore[v] = tentativeG;
                parentNode[v] = u;
                parentRoad[v] = road;
                double h = calculateHeuristicTime(v, goalId, vtype);
                pq.emplace(tentativeG + h, tentativeG, v);
            }
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    res.executionTimeMicroseconds = std::chrono::duration<double, std::micro>(endTime - startTime).count();
    res.nodesExplored = explored;

    if (gScore.find(goalId) == gScore.end()) {
        res.found = false;
        return res;
    }

    res.found = true;
    res.totalTravelTimeSec = gScore[goalId];

    // Reconstruct path
    uint32_t curr = goalId;
    while (curr != startId) {
        res.path.push_back(curr);
        RoadSegment rd = parentRoad[curr];
        res.roadSegmentIds.push_back(rd.id);
        res.totalDistanceMeters += rd.lengthMeters;
        curr = parentNode[curr];
    }
    res.path.push_back(startId);
    std::reverse(res.path.begin(), res.path.end());
    std::reverse(res.roadSegmentIds.begin(), res.roadSegmentIds.end());

    return res;
}

std::vector<RouteResult> TrafficEngine::computeKAlternativeRoutes(uint32_t startId, uint32_t goalId, size_t k, VehicleType vtype) {
    std::vector<RouteResult> results;
    RouteResult primary = computeShortestPathAStar(startId, goalId, vtype);
    if (!primary.found) return results;

    results.push_back(primary);
    if (k <= 1) return results;

    // Penalty-based diverse routing: temporarily penalize edges used in prior routes
    std::unordered_map<uint32_t, double> originalDelays;

    for (size_t iteration = 1; iteration < k; ++iteration) {
        // Apply cumulative penalties to all roads used in current results
        for (const auto& priorRoute : results) {
            for (uint32_t roadId : priorRoute.roadSegmentIds) {
                RoadSegment* rd = graph_.getRoadMutable(roadId);
                if (rd) {
                    if (originalDelays.find(roadId) == originalDelays.end()) {
                        originalDelays[roadId] = rd->incidentDelaySec;
                    }
                    rd->incidentDelaySec += 60.0 * (iteration + 1); // Synthetic penalty
                }
            }
        }

        RouteResult alternative = computeShortestPathAStar(startId, goalId, vtype);
        if (alternative.found) {
            // Recalculate true baseline cost without the artificial penalties
            double trueTravelTime = 0.0;
            for (uint32_t roadId : alternative.roadSegmentIds) {
                RoadSegment* rd = graph_.getRoadMutable(roadId);
                if (rd) {
                    double origDelay = originalDelays.count(roadId) ? originalDelays[roadId] : 0.0;
                    double savedDelay = rd->incidentDelaySec;
                    rd->incidentDelaySec = origDelay;
                    trueTravelTime += rd->getDynamicTravelTime(vtype);
                    rd->incidentDelaySec = savedDelay;
                }
            }
            alternative.totalTravelTimeSec = trueTravelTime;
            results.push_back(alternative);
        }
    }

    // Restore original delays
    for (const auto& kv : originalDelays) {
        RoadSegment* rd = graph_.getRoadMutable(kv.first);
        if (rd) {
            rd->incidentDelaySec = kv.second;
        }
    }

    return results;
}

void TrafficEngine::simulateTrafficStep(double deltaSeconds) {
    // Dynamic vehicle dispersal & signal cycle progress
    for (const auto& inter : graph_.getAllIntersections()) {
        auto& mutableInter = graph_.getIntersection(inter.id);
        if (mutableInter.signal.isAdaptive && !mutableInter.signal.isEmergencyPreempted) {
            mutableInter.signal.currentTimer += deltaSeconds;
            double targetDuration = mutableInter.signal.northSouthGreen 
                                      ? mutableInter.signal.greenDurationSec 
                                      : mutableInter.signal.redDurationSec;
            if (mutableInter.signal.currentTimer >= targetDuration) {
                mutableInter.signal.northSouthGreen = !mutableInter.signal.northSouthGreen;
                mutableInter.signal.currentTimer = 0.0;
            }
        }
    }
}

SignalTimingProfile TrafficEngine::calculateWebsterCycle(uint32_t intersectionId) {
    SignalTimingProfile profile;
    profile.intersectionId = intersectionId;

    if (!graph_.hasIntersection(intersectionId)) return profile;

    const auto& incoming = graph_.getIncomingRoads(intersectionId);
    double nsFlow = 0.0;
    double ewFlow = 0.0;
    double saturationFlow = 1800.0; // standard veh/hr per lane

    for (const auto& road : incoming) {
        // Estimate direction based on delta geometry
        const auto& fromNode = graph_.getIntersection(road.fromIntersection);
        const auto& toNode = graph_.getIntersection(road.toIntersection);

        double dy = std::abs(toNode.location.y - fromNode.location.y);
        double dx = std::abs(toNode.location.x - fromNode.location.x);

        if (dy >= dx) {
            nsFlow += road.currentVehicleFlow;
        } else {
            ewFlow += road.currentVehicleFlow;
        }
    }

    double y_ns = nsFlow / saturationFlow;
    double y_ew = ewFlow / saturationFlow;
    double Y = std::min(0.85, y_ns + y_ew); // sum of flow ratios

    // Webster's optimal cycle length: C_opt = (1.5 * L + 5) / (1 - Y)
    double lostTimeL = 8.0; // lost time in sec (change intervals)
    double cOpt = (1.5 * lostTimeL + 5.0) / std::max(0.1, 1.0 - Y);
    cOpt = std::max(30.0, std::min(120.0, cOpt)); // bound between 30s and 120s

    double effectiveGreen = cOpt - lostTimeL;
    double nsGreen = (Y > 0.001) ? (effectiveGreen * (y_ns / Y)) : (effectiveGreen / 2.0);
    double ewGreen = effectiveGreen - nsGreen;

    profile.northSouthGreenSec = std::max(10.0, nsGreen);
    profile.eastWestGreenSec = std::max(10.0, ewGreen);
    profile.totalCycleSec = cOpt;
    profile.saturationFlowRatio = Y;

    return profile;
}

void TrafficEngine::updateAdaptiveSignals() {
    for (const auto& inter : graph_.getAllIntersections()) {
        auto& mutableInter = graph_.getIntersection(inter.id);
        if (mutableInter.signal.isAdaptive && !mutableInter.signal.isCompromised && !mutableInter.signal.isEmergencyPreempted) {
            SignalTimingProfile p = calculateWebsterCycle(inter.id);
            mutableInter.signal.greenDurationSec = p.northSouthGreenSec;
            mutableInter.signal.redDurationSec = p.eastWestGreenSec;
        }
    }
}

void TrafficEngine::establishEmergencyGreenCorridor(const std::vector<uint32_t>& path) {
    for (uint32_t nodeId : path) {
        if (graph_.hasIntersection(nodeId)) {
            auto& inter = graph_.getIntersection(nodeId);
            inter.signal.isEmergencyPreempted = true;
            inter.signal.northSouthGreen = true; // Clear corridor
            inter.signal.greenDurationSec = 90.0;
        }
    }
}

void TrafficEngine::clearEmergencyPreemption(const std::vector<uint32_t>& path) {
    for (uint32_t nodeId : path) {
        if (graph_.hasIntersection(nodeId)) {
            auto& inter = graph_.getIntersection(nodeId);
            inter.signal.isEmergencyPreempted = false;
        }
    }
}

TrafficEngine::RoutingBenchmarkComparison TrafficEngine::runRoutingBenchmark(uint32_t startId, uint32_t goalId, VehicleType vtype) {
    RoutingBenchmarkComparison comp;
    comp.dijkstraResult = computeShortestPathDijkstra(startId, goalId, vtype);
    comp.aStarResult = computeShortestPathAStar(startId, goalId, vtype);

    double dTime = std::max(0.001, comp.dijkstraResult.executionTimeMicroseconds);
    double aTime = std::max(0.001, comp.aStarResult.executionTimeMicroseconds);
    comp.speedupRatio = dTime / aTime;

    if (comp.dijkstraResult.nodesExplored >= comp.aStarResult.nodesExplored) {
        comp.nodeExplorationReduction = comp.dijkstraResult.nodesExplored - comp.aStarResult.nodesExplored;
    } else {
        comp.nodeExplorationReduction = 0;
    }
    return comp;
}

} // namespace urbantis
