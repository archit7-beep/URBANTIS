#pragma once

#include "Types.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <limits>

namespace urbantis {

struct TrafficSignal {
    bool isAdaptive{true};
    double greenDurationSec{30.0};
    double redDurationSec{30.0};
    double yellowDurationSec{3.0};
    double currentTimer{0.0};
    bool northSouthGreen{true};
    bool isCompromised{false};
    bool isEmergencyPreempted{false};
    uint64_t lastAdaptiveAdjustmentTick{0};
};

struct Intersection {
    uint32_t id{0};
    std::string name;
    Point2D location;
    TrafficSignal signal;
    bool isBlocked{false};
    std::string controllingDeviceId;  // e.g. "GATEWAY-01" or "SIG-CTRL-05"
    int currentCongestionLevel{0};    // 0 = Free, 100 = Gridlock

    Intersection() = default;
    Intersection(uint32_t i, std::string n, Point2D loc, std::string devId = "")
        : id(i), name(std::move(n)), location(loc), controllingDeviceId(std::move(devId)) {}
};

struct RoadSegment {
    uint32_t id{0};
    uint32_t fromIntersection{0};
    uint32_t toIntersection{0};
    double lengthMeters{100.0};
    double speedLimitMps{13.89};       // ~50 km/h default
    double capacityVehiclesPerHour{1200.0};
    double currentVehicleFlow{100.0};  // Current vehicle count/flow
    RoadType type{RoadType::ARTERIAL};
    bool isClosed{false};
    bool isCompromised{false};         // True if manipulated by cyber attack
    double incidentDelaySec{0.0};

    // Calculate dynamic travel time using standard Bureau of Public Roads (BPR) latency model
    double getDynamicTravelTime(VehicleType vtype = VehicleType::CAR) const {
        if (isClosed) {
            return std::numeric_limits<double>::infinity();
        }

        double freeFlowTime = lengthMeters / (speedLimitMps > 0.1 ? speedLimitMps : 1.0);
        
        // Volume-to-Capacity ratio
        double vcr = currentVehicleFlow / (capacityVehiclesPerHour > 1.0 ? capacityVehiclesPerHour : 1.0);
        if (vcr < 0.0) vcr = 0.0;

        // Standard BPR: T = T0 * (1 + alpha * (V/C)^beta)
        double alpha = 0.15;
        double beta = 4.0;
        double dynamicTime = freeFlowTime * (1.0 + alpha * std::pow(vcr, beta));

        // Adjust for vehicle type
        if (vtype == VehicleType::AMBULANCE) {
            // Ambulances travel faster and can bypass partial congestion
            dynamicTime *= 0.65;
        } else if (vtype == VehicleType::BIKE) {
            // Bikes have constant lower speed limit but are unaffected by heavy car jams
            double bikeSpeed = 5.56; // 20 km/h
            double bikeTime = lengthMeters / bikeSpeed;
            return isCompromised ? (bikeTime + incidentDelaySec * 0.2) : bikeTime;
        }

        // Add incident or cyber malfunction delays
        if (isCompromised) {
            dynamicTime += 120.0; // Severe synthetic delay caused by signal malfunction/spoofing
        }
        dynamicTime += incidentDelaySec;

        return dynamicTime;
    }
};

struct CyberDevice {
    std::string id;
    CyberDeviceType type{CyberDeviceType::TRAFFIC_SIGNAL_CONTROLLER};
    DeviceOperationalStatus status{DeviceOperationalStatus::ONLINE_NORMAL};
    std::string ipAddress;
    uint32_t boundIntersectionId{0};
    std::string parentGatewayId;
    std::vector<std::string> subordinateDeviceIds;
    double anomalyScore{0.0};
    uint64_t lastHeartbeatTick{0};
};

class CityGraph {
public:
    CityGraph() = default;

    // Node & Edge insertions
    void addIntersection(const Intersection& intersection);
    void addRoad(const RoadSegment& road, bool bidirectional = true);
    void addCyberDevice(const CyberDevice& device);

    // Queries & Accessors
    bool hasIntersection(uint32_t id) const;
    const Intersection& getIntersection(uint32_t id) const;
    Intersection& getIntersection(uint32_t id);
    const std::vector<Intersection>& getAllIntersections() const { return intersections_; }

    const std::vector<RoadSegment>& getOutgoingRoads(uint32_t intersectionId) const;
    const std::vector<RoadSegment>& getIncomingRoads(uint32_t intersectionId) const;
    RoadSegment* getRoadMutable(uint32_t roadId);
    
    bool hasCyberDevice(const std::string& devId) const;
    const CyberDevice& getCyberDevice(const std::string& devId) const;
    CyberDevice& getCyberDevice(const std::string& devId);
    const std::unordered_map<std::string, CyberDevice>& getAllCyberDevices() const { return cyberDevices_; }

    // Spatial lookup
    uint32_t findNearestIntersection(const Point2D& point) const;

    // Graph Analytics & Structural Integrity (Tarjan's Articulation Points & Bridges)
    std::vector<uint32_t> findArticulationPoints() const;
    std::vector<std::pair<uint32_t, uint32_t>> findBridges() const;
    size_t countConnectedComponents() const;

    // Dynamic Traffic Modifications
    void setRoadClosed(uint32_t roadId, bool closed);
    void setIntersectionBlocked(uint32_t intersectionId, bool blocked);
    void updateRoadFlow(uint32_t roadId, double deltaVehicles);
    void setDeviceStatus(const std::string& devId, DeviceOperationalStatus status);

    // Diagnostics & Metrics
    size_t getIntersectionCount() const { return intersections_.size(); }
    size_t getRoadCount() const { return allRoads_.size(); }
    double getTotalRoadNetworkKm() const;

    // Built-in synthetic city generator (Generates realistic grid + ring highway + facilities)
    void generateSyntheticSmartCity(uint32_t gridRows = 5, uint32_t gridCols = 5);

private:
    std::vector<Intersection> intersections_;
    std::unordered_map<uint32_t, size_t> intersectionIndexMap_;

    std::vector<RoadSegment> allRoads_;
    std::vector<std::vector<RoadSegment>> adjacencyList_;
    std::vector<std::vector<RoadSegment>> reverseAdjacencyList_;

    std::unordered_map<std::string, CyberDevice> cyberDevices_;

    // Helper for Tarjan's Articulation Points
    void dfsArticulation(uint32_t u, std::vector<bool>& visited, std::vector<int>& discoveryTime,
                         std::vector<int>& lowTime, std::vector<int>& parent,
                         std::vector<bool>& isArt, int& timer) const;
};

} // namespace urbantis
