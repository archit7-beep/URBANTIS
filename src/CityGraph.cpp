#include "CityGraph.hpp"
#include <algorithm>
#include <sstream>
#include <queue>
#include <cmath>

namespace urbantis {

void CityGraph::addIntersection(const Intersection& intersection) {
    if (intersectionIndexMap_.find(intersection.id) != intersectionIndexMap_.end()) {
        intersections_[intersectionIndexMap_[intersection.id]] = intersection;
        return;
    }
    size_t index = intersections_.size();
    intersectionIndexMap_[intersection.id] = index;
    intersections_.push_back(intersection);
    adjacencyList_.emplace_back();
    reverseAdjacencyList_.emplace_back();
}

void CityGraph::addRoad(const RoadSegment& road, bool bidirectional) {
    if (!hasIntersection(road.fromIntersection) || !hasIntersection(road.toIntersection)) {
        return;
    }

    allRoads_.push_back(road);
    size_t uIdx = intersectionIndexMap_[road.fromIntersection];
    size_t vIdx = intersectionIndexMap_[road.toIntersection];

    adjacencyList_[uIdx].push_back(road);
    reverseAdjacencyList_[vIdx].push_back(road);

    if (bidirectional) {
        RoadSegment revRoad = road;
        revRoad.id = road.id + 100000;
        revRoad.fromIntersection = road.toIntersection;
        revRoad.toIntersection = road.fromIntersection;

        allRoads_.push_back(revRoad);
        adjacencyList_[vIdx].push_back(revRoad);
        reverseAdjacencyList_[uIdx].push_back(revRoad);
    }
}

void CityGraph::addCyberDevice(const CyberDevice& device) {
    cyberDevices_[device.id] = device;
}

bool CityGraph::hasIntersection(uint32_t id) const {
    return intersectionIndexMap_.find(id) != intersectionIndexMap_.end();
}

const Intersection& CityGraph::getIntersection(uint32_t id) const {
    static const Intersection nullIntersection;
    auto it = intersectionIndexMap_.find(id);
    if (it != intersectionIndexMap_.end()) {
        return intersections_[it->second];
    }
    return nullIntersection;
}

Intersection& CityGraph::getIntersection(uint32_t id) {
    auto it = intersectionIndexMap_.find(id);
    if (it != intersectionIndexMap_.end()) {
        return intersections_[it->second];
    }
    static Intersection fallback;
    return fallback;
}

const std::vector<RoadSegment>& CityGraph::getOutgoingRoads(uint32_t intersectionId) const {
    static const std::vector<RoadSegment> emptyList;
    auto it = intersectionIndexMap_.find(intersectionId);
    if (it != intersectionIndexMap_.end()) {
        return adjacencyList_[it->second];
    }
    return emptyList;
}

const std::vector<RoadSegment>& CityGraph::getIncomingRoads(uint32_t intersectionId) const {
    static const std::vector<RoadSegment> emptyList;
    auto it = intersectionIndexMap_.find(intersectionId);
    if (it != intersectionIndexMap_.end()) {
        return reverseAdjacencyList_[it->second];
    }
    return emptyList;
}

RoadSegment* CityGraph::getRoadMutable(uint32_t roadId) {
    for (auto& r : allRoads_) {
        if (r.id == roadId) return &r;
    }
    return nullptr;
}

bool CityGraph::hasCyberDevice(const std::string& devId) const {
    return cyberDevices_.find(devId) != cyberDevices_.end();
}

const CyberDevice& CityGraph::getCyberDevice(const std::string& devId) const {
    static const CyberDevice emptyDevice;
    auto it = cyberDevices_.find(devId);
    if (it != cyberDevices_.end()) {
        return it->second;
    }
    return emptyDevice;
}

CyberDevice& CityGraph::getCyberDevice(const std::string& devId) {
    auto it = cyberDevices_.find(devId);
    if (it != cyberDevices_.end()) {
        return it->second;
    }
    static CyberDevice fallback;
    return fallback;
}

uint32_t CityGraph::findNearestIntersection(const Point2D& point) const {
    if (intersections_.empty()) return 0;
    uint32_t nearestId = intersections_[0].id;
    double minDistance = std::numeric_limits<double>::max();

    for (const auto& inter : intersections_) {
        double dist = inter.location.distanceTo(point);
        if (dist < minDistance) {
            minDistance = dist;
            nearestId = inter.id;
        }
    }
    return nearestId;
}

void CityGraph::setRoadClosed(uint32_t roadId, bool closed) {
    for (auto& r : allRoads_) {
        if (r.id == roadId) {
            r.isClosed = closed;
        }
    }
    for (auto& adjList : adjacencyList_) {
        for (auto& r : adjList) {
            if (r.id == roadId) {
                r.isClosed = closed;
            }
        }
    }
    for (auto& radjList : reverseAdjacencyList_) {
        for (auto& r : radjList) {
            if (r.id == roadId) {
                r.isClosed = closed;
            }
        }
    }
}

void CityGraph::setIntersectionBlocked(uint32_t intersectionId, bool blocked) {
    if (hasIntersection(intersectionId)) {
        getIntersection(intersectionId).isBlocked = blocked;
    }
}

void CityGraph::updateRoadFlow(uint32_t roadId, double deltaVehicles) {
    for (auto& r : allRoads_) {
        if (r.id == roadId) {
            r.currentVehicleFlow = std::max(0.0, r.currentVehicleFlow + deltaVehicles);
        }
    }
    for (auto& adjList : adjacencyList_) {
        for (auto& r : adjList) {
            if (r.id == roadId) {
                r.currentVehicleFlow = std::max(0.0, r.currentVehicleFlow + deltaVehicles);
            }
        }
    }
}

void CityGraph::setDeviceStatus(const std::string& devId, DeviceOperationalStatus status) {
    auto it = cyberDevices_.find(devId);
    if (it != cyberDevices_.end()) {
        it->second.status = status;
        if (it->second.boundIntersectionId != 0 && hasIntersection(it->second.boundIntersectionId)) {
            auto& inter = getIntersection(it->second.boundIntersectionId);
            if (status == DeviceOperationalStatus::COMPROMISED) {
                inter.signal.isCompromised = true;
            } else if (status == DeviceOperationalStatus::ISOLATED) {
                inter.signal.isAdaptive = false; // Fallback to manual 4-way stop or fixed failover
                inter.signal.isCompromised = false;
            } else if (status == DeviceOperationalStatus::ONLINE_NORMAL) {
                inter.signal.isCompromised = false;
            }
        }
    }
}

double CityGraph::getTotalRoadNetworkKm() const {
    double totalMeters = 0.0;
    for (const auto& r : allRoads_) {
        totalMeters += r.lengthMeters;
    }
    return totalMeters / 1000.0;
}

void CityGraph::dfsArticulation(uint32_t u, std::vector<bool>& visited,
                               std::vector<int>& discoveryTime,
                               std::vector<int>& lowTime,
                               std::vector<int>& parent,
                               std::vector<bool>& isArt,
                               int& timer) const {
    visited[u] = true;
    discoveryTime[u] = lowTime[u] = ++timer;
    int children = 0;

    for (const auto& road : adjacencyList_[u]) {
        size_t v = intersectionIndexMap_.at(road.toIntersection);
        if (!visited[v]) {
            children++;
            parent[v] = static_cast<int>(u);
            dfsArticulation(static_cast<uint32_t>(v), visited, discoveryTime, lowTime, parent, isArt, timer);

            lowTime[u] = std::min(lowTime[u], lowTime[v]);

            if (parent[u] == -1 && children > 1) {
                isArt[u] = true;
            }
            if (parent[u] != -1 && lowTime[v] >= discoveryTime[u]) {
                isArt[u] = true;
            }
        } else if (static_cast<int>(v) != parent[u]) {
            lowTime[u] = std::min(lowTime[u], discoveryTime[v]);
        }
    }
}

std::vector<uint32_t> CityGraph::findArticulationPoints() const {
    size_t n = intersections_.size();
    std::vector<bool> visited(n, false);
    std::vector<int> discoveryTime(n, 0);
    std::vector<int> lowTime(n, 0);
    std::vector<int> parent(n, -1);
    std::vector<bool> isArt(n, false);
    int timer = 0;

    for (size_t i = 0; i < n; ++i) {
        if (!visited[i]) {
            dfsArticulation(static_cast<uint32_t>(i), visited, discoveryTime, lowTime, parent, isArt, timer);
        }
    }

    std::vector<uint32_t> result;
    for (size_t i = 0; i < n; ++i) {
        if (isArt[i]) {
            result.push_back(intersections_[i].id);
        }
    }
    return result;
}

std::vector<std::pair<uint32_t, uint32_t>> CityGraph::findBridges() const {
    size_t n = intersections_.size();
    std::vector<bool> visited(n, false);
    std::vector<int> disc(n, 0), low(n, 0), parent(n, -1);
    int timer = 0;
    std::vector<std::pair<uint32_t, uint32_t>> bridges;

    auto dfsBridge = [&](auto& self, size_t u) -> void {
        visited[u] = true;
        disc[u] = low[u] = ++timer;

        for (const auto& road : adjacencyList_[u]) {
            size_t v = intersectionIndexMap_.at(road.toIntersection);
            if (!visited[v]) {
                parent[v] = static_cast<int>(u);
                self(self, v);

                low[u] = std::min(low[u], low[v]);
                if (low[v] > disc[u]) {
                    bridges.emplace_back(intersections_[u].id, intersections_[v].id);
                }
            } else if (static_cast<int>(v) != parent[u]) {
                low[u] = std::min(low[u], disc[v]);
            }
        }
    };

    for (size_t i = 0; i < n; ++i) {
        if (!visited[i]) {
            dfsBridge(dfsBridge, i);
        }
    }
    return bridges;
}

size_t CityGraph::countConnectedComponents() const {
    size_t n = intersections_.size();
    std::vector<bool> visited(n, false);
    size_t components = 0;

    for (size_t i = 0; i < n; ++i) {
        if (!visited[i]) {
            components++;
            std::queue<size_t> q;
            q.push(i);
            visited[i] = true;

            while (!q.empty()) {
                size_t curr = q.front();
                q.pop();

                for (const auto& road : adjacencyList_[curr]) {
                    if (road.isClosed) continue;
                    size_t nextIdx = intersectionIndexMap_.at(road.toIntersection);
                    if (!visited[nextIdx] && !intersections_[nextIdx].isBlocked) {
                        visited[nextIdx] = true;
                        q.push(nextIdx);
                    }
                }
            }
        }
    }
    return components;
}

void CityGraph::generateSyntheticSmartCity(uint32_t gridRows, uint32_t gridCols) {
    (void)gridRows;
    (void)gridCols;
    intersections_.clear();
    intersectionIndexMap_.clear();
    allRoads_.clear();
    adjacencyList_.clear();
    reverseAdjacencyList_.clear();
    cyberDevices_.clear();

    struct PuneNodeDef {
        uint32_t id;
        std::string name;
        double x, y;
        std::string gateway;
    };

    std::vector<PuneNodeDef> puneNodes = {
        {1, "Pune University Circle", 500, 200, "SRV-PMC-SMARTCITY-CORE"},
        {2, "Shivajinagar / Shimla Office", 1000, 400, "SRV-PMC-SMARTCITY-CORE"},
        {3, "Pune Station / Sassoon Chowk", 1500, 500, "SRV-PMC-SMARTCITY-CORE"},
        {4, "Yerawada / Bund Garden Chowk", 1800, 200, "GW-MAGARPATTA-CYBER-02"},
        {5, "Viman Nagar / Phoenix Chowk", 2400, 100, "GW-MAGARPATTA-CYBER-02"},
        {6, "Senapati Bapat Road (ICC Towers)", 600, 500, "SRV-PMC-SMARTCITY-CORE"},
        {7, "FC Road / Deccan Gymkhana", 900, 700, "SRV-PMC-SMARTCITY-CORE"},
        {8, "JM Road / Sambhaji Bridge", 1100, 700, "SRV-PMC-SMARTCITY-CORE"},
        {9, "Camp / MG Road Chowk", 1600, 800, "GW-MAGARPATTA-CYBER-02"},
        {10, "Kalyani Nagar Chowk", 2000, 400, "GW-MAGARPATTA-CYBER-02"},
        {11, "Kothrud / Paud Phata", 400, 900, "GW-HINJEWADI-CYBER-01"},
        {12, "Deenanath Mangeshkar Chowk", 700, 1000, "GW-HINJEWADI-CYBER-01"},
        {13, "Swargate Chowk", 1200, 1100, "GW-SWARGATE-TRANSIT-03"},
        {14, "Fatima Nagar Chowk", 1800, 1000, "GW-MAGARPATTA-CYBER-02"},
        {15, "Hadapsar / Magarpatta City", 2300, 1000, "GW-MAGARPATTA-CYBER-02"},
        {16, "Chandani Chowk (NH-48)", 100, 1100, "GW-HINJEWADI-CYBER-01"},
        {17, "Warje Flyover Chowk", 300, 1400, "GW-HINJEWADI-CYBER-01"},
        {18, "Sinhagad Road / Sarasbaug", 1000, 1300, "GW-SWARGATE-TRANSIT-03"},
        {19, "Market Yard / Gultekdi", 1300, 1400, "GW-SWARGATE-TRANSIT-03"},
        {20, "Kondhwa / Lullanagar Chowk", 1700, 1400, "GW-SWARGATE-TRANSIT-03"},
        {21, "Hinjewadi Phase 1 (IT Hub)", 0, 0, "GW-HINJEWADI-CYBER-01"},
        {22, "Wakad / Dange Chowk", 200, 100, "GW-HINJEWADI-CYBER-01"},
        {23, "Baner / Balewadi High Street", 400, 300, "GW-HINJEWADI-CYBER-01"},
        {24, "Katraj Chowk (Wonder City)", 1100, 1800, "GW-SWARGATE-TRANSIT-03"},
        {25, "Bibwewadi Chowk", 1400, 1600, "GW-SWARGATE-TRANSIT-03"}
    };

    // 1. Add Pune Intersections and traffic signal controllers
    for (const auto& nd : puneNodes) {
        Point2D pos(nd.x, nd.y);
        std::string devId = "SIG-CTRL-PUNE-" + std::to_string(nd.id);
        Intersection inter(nd.id, nd.name, pos, devId);
        addIntersection(inter);

        CyberDevice dev;
        dev.id = devId;
        dev.type = CyberDeviceType::TRAFFIC_SIGNAL_CONTROLLER;
        dev.status = DeviceOperationalStatus::ONLINE_NORMAL;
        dev.ipAddress = "10.20." + std::to_string(nd.id) + ".10";
        dev.boundIntersectionId = nd.id;
        dev.parentGatewayId = nd.gateway;
        addCyberDevice(dev);
    }

    // 2. Add Regional Gateways and PMC Command Server
    CyberDevice pmcCore;
    pmcCore.id = "SRV-PMC-SMARTCITY-CORE";
    pmcCore.type = CyberDeviceType::MUNICIPAL_SERVER;
    pmcCore.ipAddress = "10.0.1.1";
    pmcCore.status = DeviceOperationalStatus::ONLINE_NORMAL;
    pmcCore.boundIntersectionId = 2; // Shivajinagar
    addCyberDevice(pmcCore);

    CyberDevice gwHinjewadi;
    gwHinjewadi.id = "GW-HINJEWADI-CYBER-01";
    gwHinjewadi.type = CyberDeviceType::GATEWAY_ROUTER;
    gwHinjewadi.ipAddress = "10.0.2.1";
    gwHinjewadi.status = DeviceOperationalStatus::ONLINE_NORMAL;
    gwHinjewadi.boundIntersectionId = 21; // Hinjewadi
    addCyberDevice(gwHinjewadi);

    CyberDevice gwMagarpatta;
    gwMagarpatta.id = "GW-MAGARPATTA-CYBER-02";
    gwMagarpatta.type = CyberDeviceType::GATEWAY_ROUTER;
    gwMagarpatta.ipAddress = "10.0.3.1";
    gwMagarpatta.status = DeviceOperationalStatus::ONLINE_NORMAL;
    gwMagarpatta.boundIntersectionId = 15; // Magarpatta City
    addCyberDevice(gwMagarpatta);

    CyberDevice gwSwargate;
    gwSwargate.id = "GW-SWARGATE-TRANSIT-03";
    gwSwargate.type = CyberDeviceType::GATEWAY_ROUTER;
    gwSwargate.ipAddress = "10.0.4.1";
    gwSwargate.status = DeviceOperationalStatus::ONLINE_NORMAL;
    gwSwargate.boundIntersectionId = 13; // Swargate
    addCyberDevice(gwSwargate);

    // 3. Connect Pune Arterial Corridors & Express Highway links
    uint32_t roadIdCounter = 1;
    auto addCorridor = [&](uint32_t u, uint32_t v, RoadType rtype, double speedKmH, double cap) {
        RoadSegment road;
        road.id = roadIdCounter++;
        road.fromIntersection = u;
        road.toIntersection = v;
        road.lengthMeters = getIntersection(u).location.distanceTo(getIntersection(v).location);
        road.speedLimitMps = speedKmH / 3.6;
        road.capacityVehiclesPerHour = cap;
        road.currentVehicleFlow = cap * 0.35; // 35% typical baseline load
        road.type = rtype;
        addRoad(road, true);
    };

    // Major Arterials across Pune:
    // IT Corridor: Hinjewadi -> Wakad -> Baner -> Pune University
    addCorridor(21, 22, RoadType::HIGHWAY, 60.0, 2400.0);
    addCorridor(22, 23, RoadType::ARTERIAL, 50.0, 1800.0);
    addCorridor(23, 1, RoadType::ARTERIAL, 50.0, 1800.0);

    // University -> Shivajinagar -> Station
    addCorridor(1, 2, RoadType::ARTERIAL, 50.0, 2000.0);
    addCorridor(2, 3, RoadType::ARTERIAL, 50.0, 2200.0);

    // S.B. Road & Deccan/FC/JM Road Heart
    addCorridor(1, 6, RoadType::ARTERIAL, 45.0, 1400.0);
    addCorridor(6, 7, RoadType::LOCAL, 40.0, 1200.0);
    addCorridor(2, 8, RoadType::ARTERIAL, 45.0, 1600.0);
    addCorridor(7, 8, RoadType::CONNECTOR, 35.0, 1000.0);

    // Station -> Bund Garden -> Kalyani Nagar -> Viman Nagar
    addCorridor(3, 4, RoadType::ARTERIAL, 50.0, 2000.0);
    addCorridor(4, 10, RoadType::ARTERIAL, 45.0, 1600.0);
    addCorridor(10, 5, RoadType::ARTERIAL, 50.0, 1800.0);

    // Station -> Camp -> Fatima Nagar -> Magarpatta City (Hadapsar)
    addCorridor(3, 9, RoadType::ARTERIAL, 45.0, 1600.0);
    addCorridor(9, 14, RoadType::ARTERIAL, 45.0, 1600.0);
    addCorridor(14, 15, RoadType::ARTERIAL, 50.0, 2000.0);
    addCorridor(10, 15, RoadType::CONNECTOR, 40.0, 1200.0);

    // Kothrud & Karve Road: Kothrud -> Deenanath -> Deccan
    addCorridor(11, 12, RoadType::ARTERIAL, 45.0, 1500.0);
    addCorridor(12, 7, RoadType::ARTERIAL, 40.0, 1400.0);

    // Central Deccan / JM -> Swargate Hub
    addCorridor(8, 13, RoadType::ARTERIAL, 50.0, 2200.0);
    addCorridor(7, 13, RoadType::ARTERIAL, 45.0, 1800.0);

    // Swargate -> Sarasbaug / Market Yard / Kondhwa
    addCorridor(13, 18, RoadType::LOCAL, 40.0, 1400.0);
    addCorridor(13, 19, RoadType::ARTERIAL, 45.0, 1600.0);
    addCorridor(19, 20, RoadType::LOCAL, 40.0, 1300.0);
    addCorridor(14, 20, RoadType::CONNECTOR, 40.0, 1200.0);

    // South Sector: Swargate -> Bibwewadi -> Katraj
    addCorridor(13, 25, RoadType::ARTERIAL, 45.0, 1600.0);
    addCorridor(19, 25, RoadType::LOCAL, 40.0, 1200.0);
    addCorridor(25, 24, RoadType::ARTERIAL, 50.0, 2000.0);
    addCorridor(18, 24, RoadType::ARTERIAL, 45.0, 1500.0);

    // Mumbai-Bangalore Highway Bypass (NH-48 Western Ring Road)
    // Hinjewadi -> Wakad -> Baner -> Chandani Chowk -> Warje -> Katraj
    addCorridor(21, 16, RoadType::HIGHWAY, 80.0, 3200.0);
    addCorridor(11, 16, RoadType::ARTERIAL, 50.0, 1600.0);
    addCorridor(16, 17, RoadType::HIGHWAY, 80.0, 3200.0);
    addCorridor(17, 24, RoadType::HIGHWAY, 80.0, 3200.0);
}

} // namespace urbantis
