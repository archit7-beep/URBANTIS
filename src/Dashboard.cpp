#include "Dashboard.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>

namespace urbantis {

namespace {
    const std::string RESET   = "\033[0m";
    const std::string BOLD    = "\033[1m";
    const std::string RED     = "\033[31m";
    const std::string GREEN   = "\033[32m";
    const std::string YELLOW  = "\033[33m";
    const std::string BLUE    = "\033[34m";
    const std::string MAGENTA = "\033[35m";
    const std::string CYAN    = "\033[36m";
    const std::string WHITE   = "\033[37m";
    const std::string GRAY    = "\033[90m";
    const std::string BRED    = "\033[91m";
    const std::string BGREEN  = "\033[92m";
    const std::string BCYAN   = "\033[96m";
}

std::string Dashboard::getIntersectionGlyph(const CityGraph& graph, uint32_t intersectionId,
                                            const std::vector<Ambulance>& ambulances) const {
    // Check if an ambulance is at this node
    for (const auto& amb : ambulances) {
        if (!amb.isAvailable && amb.currentIntersectionId == intersectionId) {
            return BOLD + MAGENTA + "[@AMB]" + RESET;
        }
    }

    if (!graph.hasIntersection(intersectionId)) {
        return GRAY + "[ ???]" + RESET;
    }

    const auto& inter = graph.getIntersection(intersectionId);
    if (inter.isBlocked) {
        return BOLD + RED + "[!BLK]" + RESET;
    }
    if (inter.signal.isCompromised) {
        return BOLD + BRED + "[X-CYB]" + RESET;
    }
    if (inter.signal.isEmergencyPreempted) {
        return BOLD + BCYAN + "[*EMG]" + RESET;
    }

    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << intersectionId;
    std::string idStr = oss.str();

    if (inter.signal.northSouthGreen) {
        return GREEN + "[+" + idStr + "]" + RESET;
    } else {
        return YELLOW + "[~" + idStr + "]" + RESET;
    }
}

void Dashboard::renderCityMap(const SimulationEngine& engine) {
    const auto& graph = engine.getGraph();
    const auto& ambulances = engine.getEmergencyManager().getAllAmbulances();

    std::cout << BOLD << CYAN << "=== SMART CITY DUAL-TOPOLOGY GRID (5x5 Intersections + Ring Highway) ===" << RESET << "\n";
    std::cout << GRAY << "Legend: " << GREEN << "[+ID] NS-Green" << GRAY << " | "
              << YELLOW << "[~ID] EW-Green" << GRAY << " | "
              << MAGENTA << "[@AMB] Ambulance" << GRAY << " | "
              << BCYAN << "[*EMG] Preempted Corridor" << GRAY << " | "
              << RED << "[X-CYB] Cyber-Compromised" << RESET << "\n\n";

    uint32_t rows = 5;
    uint32_t cols = 5;

    for (uint32_t r = 0; r < rows; ++r) {
        std::cout << "   ";
        for (uint32_t c = 0; c < cols; ++c) {
            uint32_t id = r * cols + c + 1;
            std::cout << getIntersectionGlyph(graph, id, ambulances);
            if (c + 1 < cols) {
                std::cout << GRAY << "=======" << RESET;
            }
        }
        std::cout << "\n";

        if (r + 1 < rows) {
            std::cout << "     ";
            for (uint32_t c = 0; c < cols; ++c) {
                std::cout << GRAY << "  ||  " << RESET;
                if (c + 1 < cols) {
                    std::cout << "       ";
                }
            }
            std::cout << "\n";
        }
    }
    std::cout << "\n";
}

void Dashboard::renderThreatGraphFeed(const SimulationEngine& engine) {
    const auto& detector = engine.getThreatDetector();
    auto rankedAlerts = detector.getRankedAlerts();

    std::cout << BOLD << RED << "=== THREATGRAPH CYBERSECURITY ENGINE (MIDAS + Killchain Trie) ===" << RESET << "\n";
    std::cout << " Total Events Ingested: " << BOLD << detector.getEventsProcessedCount() << RESET
              << " | Active Alerts: " << BOLD << rankedAlerts.size() << RESET << "\n";

    if (rankedAlerts.empty()) {
        std::cout << BGREEN << " [OK] All network segments operating normally. Zero anomalies detected." << RESET << "\n\n";
    } else {
        std::cout << GRAY << "--------------------------------------------------------------------------------------------------" << RESET << "\n";
        std::cout << BOLD << " SEV | ANOMALY | CATEGORY                       | TARGET       | FORENSIC EXPLANATION" << RESET << "\n";
        std::cout << GRAY << "--------------------------------------------------------------------------------------------------" << RESET << "\n";

        size_t shown = 0;
        for (const auto& alert : rankedAlerts) {
            if (shown++ >= 4) break; // Top 4 alerts

            std::string sevColor = (alert.severityLevel >= 5) ? BRED : (alert.severityLevel >= 4 ? RED : YELLOW);
            std::cout << " " << sevColor << BOLD << "[LV" << alert.severityLevel << "]" << RESET << " "
                      << std::fixed << std::setprecision(1) << std::setw(7) << alert.anomalyScore << " | "
                      << std::left << std::setw(30) << alert.category.substr(0, 30) << " | "
                      << std::setw(12) << alert.targetDeviceId << " | "
                      << alert.explanation.substr(0, 50) << "...\n";
        }
        std::cout << GRAY << "--------------------------------------------------------------------------------------------------" << RESET << "\n\n";
    }
}

void Dashboard::renderEmergencyStatus(const SimulationEngine& engine) {
    const auto& emg = engine.getEmergencyManager();
    const auto& hospitals = emg.getAllHospitals();
    const auto& ambulances = emg.getAllAmbulances();
    const auto& activeInc = emg.getActiveIncidents();

    std::cout << BOLD << MAGENTA << "=== EMERGENCY RESPONSE & HOSPITAL TRIAGE COORDINATION ===" << RESET << "\n";
    std::cout << " Active Emergency Incidents: " << BOLD << activeInc.size() << RESET
              << " | Resolved: " << BOLD << emg.getSuccessfulDispatches() << RESET
              << " | Avg Response Time: " << BOLD << std::fixed << std::setprecision(1) << emg.getAverageResponseTimeSec() << "s" << RESET << "\n";

    std::cout << GRAY << " Ambulances: " << RESET;
    for (const auto& a : ambulances) {
        std::string st = a.isAvailable ? (BGREEN + "IDLE@" + std::to_string(a.currentIntersectionId) + RESET)
                                       : (BRED + "BUSY->INC#" + std::to_string(a.currentEmergencyId) + RESET);
        std::cout << "[" << a.name << ": " << st << "] ";
    }
    std::cout << "\n";

    std::cout << GRAY << " Hospitals:  " << RESET;
    for (const auto& h : hospitals) {
        std::cout << "[" << h.name << " (L" << h.traumaLevel << ") Beds: " << h.availableBeds << "/" << h.totalBeds << "] ";
    }
    std::cout << "\n\n";
}

void Dashboard::renderDeliveryMetrics(const SimulationEngine& engine) {
    const auto& sa = engine.getServiceAssignment();
    std::cout << BOLD << YELLOW << "=== COURIER & PUBLIC SERVICE ASSIGNMENT (Bipartite + 2-Opt) ===" << RESET << "\n";
    std::cout << " Pending Requests: " << BOLD << sa.getPendingRequests().size() << RESET
              << " | Total Completed: " << BOLD << sa.getTotalCompletedDeliveries() << RESET
              << " | On-Time SLA Rate: " << BGREEN << BOLD << std::fixed << std::setprecision(1)
              << sa.getOverallOnTimeDeliveryRate() << "%" << RESET << "\n\n";
}

void Dashboard::render(const SimulationEngine& engine) {
    std::cout << "\033[2J\033[H"; // Clear screen and home cursor
    std::cout << BOLD << BCYAN << "==================================================================================================" << RESET << "\n";
    std::cout << BOLD << WHITE << "                          URBANTIS: SMART CITY CYBER-RESILIENT SIMULATOR                          " << RESET << "\n";
    std::cout << GRAY << " Tick: " << engine.getCurrentTick() << " | Sim Time: " << engine.getSimulationTimeSec()
              << "s | Road Network: " << engine.getGraph().getTotalRoadNetworkKm() << " km | Status: RUNNING" << RESET << "\n";
    std::cout << BOLD << BCYAN << "==================================================================================================" << RESET << "\n\n";

    renderCityMap(engine);
    renderThreatGraphFeed(engine);
    renderEmergencyStatus(engine);
    renderDeliveryMetrics(engine);
}

void Dashboard::renderBenchmarkReport(const BenchmarkSuiteResults& bench) {
    std::cout << "\n" << BOLD << BCYAN << "==================================================================================================" << RESET << "\n";
    std::cout << BOLD << WHITE << "                       URBANTIS ALGORITHMIC BENCHMARK & PERFORMANCE LAB                           " << RESET << "\n";
    std::cout << BOLD << BCYAN << "==================================================================================================" << RESET << "\n\n";

    std::cout << BOLD << "1. TRAFFIC ROUTING: DIJKSTRA VS A* HEURISTIC SEARCH" << RESET << "\n";
    std::cout << "   - Binary-Heap Dijkstra Average Runtime: " << std::fixed << std::setprecision(2) << bench.dijkstraAvgMicros << " us\n";
    std::cout << "   - Consistent A* Heuristic Runtime:      " << std::fixed << std::setprecision(2) << bench.aStarAvgMicros << " us\n";
    std::cout << "   - " << BGREEN << BOLD << "Speedup Multiplier:                      " << bench.aStarSpeedupMultiplier << "x FASTER" << RESET << "\n";
    std::cout << "   - " << BGREEN << BOLD << "Node Exploration Reduction:             " << bench.nodeExplorationReductionPercent << "% FEWER NODES" << RESET << "\n\n";

    std::cout << BOLD << "2. SERVICE ALLOCATION: GREEDY VS OPTIMAL BIPARTITE MATCHING" << RESET << "\n";
    std::cout << "   - Greedy First-Available Feasibility:   " << std::fixed << std::setprecision(1) << bench.greedyFeasibilityPercent << "%\n";
    std::cout << "   - Optimal Bipartite Match Feasibility:  " << std::fixed << std::setprecision(1) << bench.optimalFeasibilityPercent << "%\n";
    std::cout << "   - " << BGREEN << BOLD << "Transit Delay Reduction:                " << bench.travelTimeReductionPercent << "% LESS TRAVEL TIME" << RESET << "\n\n";

    std::cout << BOLD << "3. THREATGRAPH STREAMING ANOMALY DETECTION (MIDAS ENGINE)" << RESET << "\n";
    std::cout << "   - " << BGREEN << BOLD << "Streaming Event Throughput:             " << std::fixed << std::setprecision(0)
              << bench.threatDetectionThroughputEventsPerSec << " events/second" << RESET << "\n";
    std::cout << "   - Memory Complexity:                    O(1) Constant (Count-Min Dynamic Sketch)\n";
    std::cout << "   - Real-Time Anomaly Alerts Generated:   " << bench.alertsGenerated << "\n\n";

    std::cout << BOLD << "4. MULTI-OBJECTIVE INCIDENT RESPONSE PARETO EVALUATION" << RESET << "\n";
    std::cout << "   Strategy 1: Passive Monitor:            Cost = " << std::fixed << std::setprecision(1) << bench.passiveMonitorScore << "\n";
    std::cout << "   Strategy 2: Aggressive Full Isolation:  Cost = " << std::fixed << std::setprecision(1) << bench.aggressiveIsolationScore << "\n";
    std::cout << "   Strategy 3: Surgical Gateway Failover:  Cost = " << std::fixed << std::setprecision(1) << bench.surgicalFailoverScore << "\n";
    std::cout << "   Strategy 4: Emergency Green Corridor:   Cost = " << std::fixed << std::setprecision(1) << bench.emergencyCorridorScore << "\n";
    std::cout << "   - " << BGREEN << BOLD << "Optimal Selected Strategy:             " << bench.recommendedStrategy
              << " (Minimizes Composite Loss)" << RESET << "\n";

    std::cout << BOLD << BCYAN << "==================================================================================================" << RESET << "\n\n";
}

void Dashboard::printHelpMenu() {
    std::cout << BOLD << "Available Simulation Commands:" << RESET << "\n"
              << " [1] Step Simulation (1 tick)\n"
              << " [2] Run 10 ticks\n"
              << " [3] Inject DDoS Attack on Gateway (GW-NORTH-01)\n"
              << " [4] Inject Staged Kill-Chain on Traffic Controller (SIG-CTRL-13)\n"
              << " [5] Inject Traffic Accident & Emergency 911 Call (Intersection 14)\n"
              << " [6] Submit Random Delivery Orders (Food/Medicine)\n"
              << " [7] Run Head-to-Head Algorithm Benchmark Lab\n"
              << " [8] Run Autonomous Scenario Showcase (All Features)\n"
              << " [0] Exit\n"
              << "Enter choice: ";
}

} // namespace urbantis
