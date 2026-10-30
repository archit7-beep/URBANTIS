#include "CityGraph.hpp"
#include "TrafficEngine.hpp"
#include "ServiceAssignment.hpp"
#include "EmergencyManager.hpp"
#include "ThreatDetector.hpp"
#include "ImpactAnalyzer.hpp"
#include "ResponseOptimizer.hpp"
#include <iostream>
#include <cassert>

using namespace urbantis;

void testCityGraph() {
    std::cout << "[TEST] Testing CityGraph & Tarjan Articulation Points...\n";
    CityGraph graph;
    graph.generateSyntheticSmartCity(5, 5);

    assert(graph.getIntersectionCount() == 25);
    assert(graph.getRoadCount() > 40);
    assert(graph.countConnectedComponents() == 1);

    auto artPoints = graph.findArticulationPoints();
    std::cout << "  -> Found " << artPoints.size() << " articulation bottlenecks.\n";
    std::cout << "  -> City road network: " << graph.getTotalRoadNetworkKm() << " km.\n";
    std::cout << "  [PASS] CityGraph tests passed.\n\n";
}

void testRoutingAlgorithms() {
    std::cout << "[TEST] Testing Dijkstra vs A* Shortest Paths...\n";
    CityGraph graph;
    graph.generateSyntheticSmartCity(5, 5);
    TrafficEngine engine(graph);

    uint32_t start = 1;
    uint32_t goal = 25;

    RouteResult dij = engine.computeShortestPathDijkstra(start, goal, VehicleType::CAR);
    RouteResult aStar = engine.computeShortestPathAStar(start, goal, VehicleType::CAR);

    assert(dij.found);
    assert(aStar.found);
    assert(std::abs(dij.totalTravelTimeSec - aStar.totalTravelTimeSec) < 0.01);
    assert(aStar.nodesExplored <= dij.nodesExplored);

    std::cout << "  -> Dijkstra Time: " << dij.totalTravelTimeSec << "s, Explored: " << dij.nodesExplored << " nodes.\n";
    std::cout << "  -> A* Heuristic:  " << aStar.totalTravelTimeSec << "s, Explored: " << aStar.nodesExplored << " nodes.\n";
    std::cout << "  [PASS] Routing algorithms tests passed.\n\n";
}

void testThreatGraphDetection() {
    std::cout << "[TEST] Testing ThreatDetector (MIDAS Streaming + Killchain Trie)...\n";
    ThreatDetector detector(10.0);

    // Ingest baseline traffic
    for (uint64_t t = 1; t <= 10; ++t) {
        auto normal = detector.generateNormalTrafficStream(t, 20);
        for (const auto& ev : normal) {
            detector.ingestNetworkEvent(ev);
        }
    }
    size_t normalAlerts = detector.getActiveAlerts().size();
    std::cout << "  -> Baseline normal traffic alerts: " << normalAlerts << " (expected 0 or very low)\n";

    // Ingest DDoS flood
    auto flood = detector.generateDDoSFloodStream(11, "GW-NORTH-01", 150);
    for (const auto& ev : flood) {
        detector.ingestNetworkEvent(ev);
    }
    size_t afterFloodAlerts = detector.getActiveAlerts().size();
    assert(afterFloodAlerts > normalAlerts);
    std::cout << "  -> Alerts after flood: " << afterFloodAlerts << " (Microcluster anomaly detected!)\n";

    // Ingest Killchain
    auto killchain = detector.generateKillchainExploitStream(12, "10.200.0.99", "SIG-CTRL-05");
    for (const auto& ev : killchain) {
        detector.ingestNetworkEvent(ev);
    }
    auto ranked = detector.getRankedAlerts();
    bool killchainFound = false;
    for (const auto& a : ranked) {
        if (a.category.find("Killchain") != std::string::npos) {
            killchainFound = true;
            break;
        }
    }
    assert(killchainFound);
    std::cout << "  -> Staged Killchain sequence correctly identified by Trie!\n";
    std::cout << "  [PASS] ThreatDetector tests passed.\n\n";
}

void testImpactAndOptimization() {
    std::cout << "[TEST] Testing ImpactAnalyzer & ResponseOptimizer...\n";
    CityGraph graph;
    graph.generateSyntheticSmartCity(5, 5);
    TrafficEngine te(graph);
    EmergencyManager em(graph, te);
    ImpactAnalyzer ia(graph);
    ResponseOptimizer ro(graph, te, em, ia);

    ImpactAssessment impact = ia.evaluateDeviceCompromise("GW-HINJEWADI-CYBER-01");
    assert(!impact.directlyAffectedIntersections.empty());
    std::cout << "  -> Compromise of GW-HINJEWADI-CYBER-01 affects " << impact.directlyAffectedIntersections.size() 
              << " intersections. Severity: " << impact.overallDisruptionSeverityScore << "/100\n";

    CyberAlert testAlert;
    testAlert.alertId = 1;
    testAlert.severityLevel = 5;
    testAlert.anomalyScore = 55.0;
    testAlert.targetDeviceId = "GW-HINJEWADI-CYBER-01";

    auto plans = ro.evaluateAllStrategies(impact, testAlert);
    assert(plans.size() == 4);
    std::cout << "  -> Evaluated " << plans.size() << " candidate response strategies.\n";
    std::cout << "  -> Top Recommended Strategy: " << strategyToString(plans.front().strategy) 
              << " with score " << plans.front().totalCompositeScore << "\n";
    std::cout << "  [PASS] Impact & Optimization tests passed.\n\n";
}

int main() {
    std::cout << "====================================================\n";
    std::cout << "        URBANTIS TEST SUITE & VERIFICATION          \n";
    std::cout << "====================================================\n\n";

    testCityGraph();
    testRoutingAlgorithms();
    testThreatGraphDetection();
    testImpactAndOptimization();

    std::cout << "====================================================\n";
    std::cout << "   ALL URBANTIS UNIT & BENCHMARK TESTS PASSED!      \n";
    std::cout << "====================================================\n";
    return 0;
}
