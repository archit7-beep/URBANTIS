#pragma once

#include "Types.hpp"
#include "CityGraph.hpp"
#include "TrafficEngine.hpp"
#include "EmergencyManager.hpp"
#include "ImpactAnalyzer.hpp"
#include <vector>
#include <memory>

namespace urbantis {

struct OptimizerWeights {
    double trafficDelayWeight{1.0};
    double emergencyRiskWeight{2.5};
    double cyberExposureWeight{2.0};
    double isolationPenaltyWeight{0.8};
};

class ResponseOptimizer {
public:
    ResponseOptimizer(CityGraph& graph, TrafficEngine& trafficEngine,
                      EmergencyManager& emergencyManager, ImpactAnalyzer& impactAnalyzer);

    void setObjectiveWeights(const OptimizerWeights& weights) { weights_ = weights; }
    const OptimizerWeights& getObjectiveWeights() const { return weights_; }

    // Multi-Criteria Evaluation of Candidate Responses
    std::vector<ResponseEvaluation> evaluateAllStrategies(const ImpactAssessment& assessment, const CyberAlert& alert);

    // Pick and execute the optimal strategy
    ResponseEvaluation selectAndApplyOptimalStrategy(const ImpactAssessment& assessment, const CyberAlert& alert);

    // Apply specific response strategy to live simulation
    void applyStrategy(const ResponseEvaluation& plan);

    // Revert strategy interventions
    void revertStrategy(const ResponseEvaluation& plan);

private:
    CityGraph& graph_;
    TrafficEngine& trafficEngine_;
    EmergencyManager& emergencyManager_;
    ImpactAnalyzer& impactAnalyzer_;
    OptimizerWeights weights_;

    // Evaluator helpers for each strategy
    ResponseEvaluation evaluatePassiveMonitor(const ImpactAssessment& assessment, const CyberAlert& alert);
    ResponseEvaluation evaluateAggressiveIsolation(const ImpactAssessment& assessment, const CyberAlert& alert);
    ResponseEvaluation evaluateSurgicalFailover(const ImpactAssessment& assessment, const CyberAlert& alert);
    ResponseEvaluation evaluateEmergencyCorridor(const ImpactAssessment& assessment, const CyberAlert& alert);
};

} // namespace urbantis
