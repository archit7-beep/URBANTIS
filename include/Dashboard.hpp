#pragma once

#include "SimulationEngine.hpp"
#include <string>

namespace urbantis {

class Dashboard {
public:
    Dashboard() = default;

    // Render Full Live Simulation View
    void render(const SimulationEngine& engine);

    // Render ASCII City Network Topology Map
    void renderCityMap(const SimulationEngine& engine);

    // Render Subsystem Status Panes
    void renderThreatGraphFeed(const SimulationEngine& engine);
    void renderEmergencyStatus(const SimulationEngine& engine);
    void renderDeliveryMetrics(const SimulationEngine& engine);

    // Render Full Algorithmic Benchmark Suite
    void renderBenchmarkReport(const BenchmarkSuiteResults& bench);

    // Interactive Menu
    void printHelpMenu();

private:
    std::string getIntersectionGlyph(const CityGraph& graph, uint32_t intersectionId,
                                    const std::vector<Ambulance>& ambulances) const;
};

} // namespace urbantis
