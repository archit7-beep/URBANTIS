#pragma once

#include "Types.hpp"
#include "CityGraph.hpp"
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace urbantis {

struct ImpactAssessment {
    std::string incidentDeviceId;
    std::vector<uint32_t> directlyAffectedIntersections;
    std::vector<uint32_t> secondaryCascadingIntersections;
    std::vector<std::pair<uint32_t, uint32_t>> disconnectedCorridors;
    bool hospitalAccessImpaired{false};
    bool articulationPointCompromised{false};
    double estimatedDelayIncreasePercent{0.0};
    double overallDisruptionSeverityScore{0.0}; // 0 - 100 scale
    std::string impactSummary;
};

class ImpactAnalyzer {
public:
    explicit ImpactAnalyzer(CityGraph& graph);

    // Dependency Mapping
    void registerDependency(const std::string& parentDeviceId, const std::string& dependentDeviceId);
    void mapDeviceToIntersection(const std::string& deviceId, uint32_t intersectionId);

    // Impact Analysis & Cascading Failure Simulation
    ImpactAssessment evaluateDeviceCompromise(const std::string& compromisedDeviceId);
    ImpactAssessment evaluateAlertImpact(const CyberAlert& alert);

    // Single Point of Failure (SPOF) Analysis
    std::vector<std::string> identifyCriticalCyberGateways() const;

private:
    CityGraph& graph_;
    std::unordered_map<std::string, std::vector<std::string>> deviceChildrenMap_;
    std::unordered_map<std::string, std::string> deviceParentMap_;
    std::unordered_map<std::string, uint32_t> deviceIntersectionMap_;
};

} // namespace urbantis
