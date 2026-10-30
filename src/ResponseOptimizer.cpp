#include "ResponseOptimizer.hpp"
#include <algorithm>
#include <sstream>

namespace urbantis {

ResponseOptimizer::ResponseOptimizer(CityGraph& graph, TrafficEngine& trafficEngine,
                                     EmergencyManager& emergencyManager, ImpactAnalyzer& impactAnalyzer)
    : graph_(graph), trafficEngine_(trafficEngine),
      emergencyManager_(emergencyManager), impactAnalyzer_(impactAnalyzer) {}

ResponseEvaluation ResponseOptimizer::evaluatePassiveMonitor(const ImpactAssessment& assessment, const CyberAlert& alert) {
    ResponseEvaluation eval;
    eval.strategy = ResponseStrategyType::PASSIVE_MONITOR;

    eval.trafficDelayCost = assessment.estimatedDelayIncreasePercent * 18.0;
    eval.emergencyRiskCost = assessment.hospitalAccessImpaired ? 500.0 : 180.0;
    eval.cyberExposureCost = alert.anomalyScore * 22.0 * alert.severityLevel;
    eval.isolationPenaltyCost = 0.0;

    eval.totalCompositeScore = (eval.trafficDelayCost * weights_.trafficDelayWeight) +
                               (eval.emergencyRiskCost * weights_.emergencyRiskWeight) +
                               (eval.cyberExposureCost * weights_.cyberExposureWeight) +
                               (eval.isolationPenaltyCost * weights_.isolationPenaltyWeight);

    eval.recommendationRationale = "Take no immediate physical action; continue monitoring event stream. Zero service disruption but high ongoing cybersecurity vulnerability and cascading traffic delay.";
    return eval;
}

ResponseEvaluation ResponseOptimizer::evaluateAggressiveIsolation(const ImpactAssessment& assessment, const CyberAlert& alert) {
    ResponseEvaluation eval;
    eval.strategy = ResponseStrategyType::AGGRESSIVE_ISOLATION;

    eval.trafficDelayCost = assessment.directlyAffectedIntersections.size() * 110.0 + 90.0;
    eval.emergencyRiskCost = assessment.hospitalAccessImpaired ? 320.0 : 60.0;
    eval.cyberExposureCost = 0.0; // Threat neutralized by hard isolation
    eval.isolationPenaltyCost = assessment.directlyAffectedIntersections.size() * 95.0;

    eval.totalCompositeScore = (eval.trafficDelayCost * weights_.trafficDelayWeight) +
                               (eval.emergencyRiskCost * weights_.emergencyRiskWeight) +
                               (eval.cyberExposureCost * weights_.cyberExposureWeight) +
                               (eval.isolationPenaltyCost * weights_.isolationPenaltyWeight);

    eval.isolatedDevices.push_back(alert.targetDeviceId.empty() ? alert.sourceDeviceId : alert.targetDeviceId);
    for (uint32_t id : assessment.directlyAffectedIntersections) {
        eval.isolatedIntersections.push_back(id);
    }

    eval.recommendationRationale = "Completely disconnect target device and put dependent intersections in 4-way stop mode. Eliminates cyber exposure entirely, but inflicts severe local travel delay and isolation penalties.";
    return eval;
}

ResponseEvaluation ResponseOptimizer::evaluateSurgicalFailover(const ImpactAssessment& assessment, const CyberAlert& alert) {
    ResponseEvaluation eval;
    eval.strategy = ResponseStrategyType::SURGICAL_FAILOVER;

    eval.trafficDelayCost = assessment.directlyAffectedIntersections.size() * 28.0;
    eval.emergencyRiskCost = assessment.hospitalAccessImpaired ? 95.0 : 25.0;
    eval.cyberExposureCost = alert.severityLevel * 18.0; // Minimal residual risk while failover locks down firmware
    eval.isolationPenaltyCost = 35.0;

    eval.totalCompositeScore = (eval.trafficDelayCost * weights_.trafficDelayWeight) +
                               (eval.emergencyRiskCost * weights_.emergencyRiskWeight) +
                               (eval.cyberExposureCost * weights_.cyberExposureWeight) +
                               (eval.isolationPenaltyCost * weights_.isolationPenaltyWeight);

    eval.isolatedDevices.push_back(alert.targetDeviceId.empty() ? alert.sourceDeviceId : alert.targetDeviceId);
    eval.recommendationRationale = "Migrate control to regional backup gateway, revert intersection signal schedule to hardcoded Webster baseline, and reroute incoming traffic around sector perimeter.";
    return eval;
}

ResponseEvaluation ResponseOptimizer::evaluateEmergencyCorridor(const ImpactAssessment& assessment, const CyberAlert& alert) {
    ResponseEvaluation eval;
    eval.strategy = ResponseStrategyType::EMERGENCY_GREEN_CORRIDOR;

    eval.trafficDelayCost = assessment.directlyAffectedIntersections.size() * 32.0;
    eval.emergencyRiskCost = 8.0; // Extremely low: priority green wave guarantees ambulance transit
    eval.cyberExposureCost = alert.severityLevel * 12.0;
    eval.isolationPenaltyCost = 40.0;

    eval.totalCompositeScore = (eval.trafficDelayCost * weights_.trafficDelayWeight) +
                               (eval.emergencyRiskCost * weights_.emergencyRiskWeight) +
                               (eval.cyberExposureCost * weights_.cyberExposureWeight) +
                               (eval.isolationPenaltyCost * weights_.isolationPenaltyWeight);

    eval.isolatedDevices.push_back(alert.targetDeviceId.empty() ? alert.sourceDeviceId : alert.targetDeviceId);
    for (uint32_t id : assessment.directlyAffectedIntersections) {
        eval.isolatedIntersections.push_back(id);
    }

    eval.recommendationRationale = "Isolate compromised controller, establish pre-cleared dynamic emergency green wave around sector, and divert general traffic to parallel arterial corridors.";
    return eval;
}

std::vector<ResponseEvaluation> ResponseOptimizer::evaluateAllStrategies(const ImpactAssessment& assessment, const CyberAlert& alert) {
    std::vector<ResponseEvaluation> plans;
    plans.push_back(evaluatePassiveMonitor(assessment, alert));
    plans.push_back(evaluateAggressiveIsolation(assessment, alert));
    plans.push_back(evaluateSurgicalFailover(assessment, alert));
    plans.push_back(evaluateEmergencyCorridor(assessment, alert));

    std::sort(plans.begin(), plans.end(),
        [](const ResponseEvaluation& a, const ResponseEvaluation& b) {
            return a.totalCompositeScore < b.totalCompositeScore;
        });

    return plans;
}

ResponseEvaluation ResponseOptimizer::selectAndApplyOptimalStrategy(const ImpactAssessment& assessment, const CyberAlert& alert) {
    std::vector<ResponseEvaluation> plans = evaluateAllStrategies(assessment, alert);
    ResponseEvaluation best = plans.front();
    applyStrategy(best);
    return best;
}

void ResponseOptimizer::applyStrategy(const ResponseEvaluation& plan) {
    switch (plan.strategy) {
        case ResponseStrategyType::PASSIVE_MONITOR:
            // Do not alter physical state
            break;

        case ResponseStrategyType::AGGRESSIVE_ISOLATION:
            for (const auto& devId : plan.isolatedDevices) {
                graph_.setDeviceStatus(devId, DeviceOperationalStatus::ISOLATED);
            }
            for (uint32_t node : plan.isolatedIntersections) {
                graph_.setIntersectionBlocked(node, true);
            }
            break;

        case ResponseStrategyType::SURGICAL_FAILOVER:
            for (const auto& devId : plan.isolatedDevices) {
                graph_.setDeviceStatus(devId, DeviceOperationalStatus::FAILOVER_MODE);
            }
            // Recalculate adaptive signals to safe Webster fallback
            trafficEngine_.updateAdaptiveSignals();
            break;

        case ResponseStrategyType::EMERGENCY_GREEN_CORRIDOR:
            for (const auto& devId : plan.isolatedDevices) {
                graph_.setDeviceStatus(devId, DeviceOperationalStatus::ISOLATED);
            }
            // For any active ambulances, re-evaluate and enforce dynamic green corridor
            for (auto& amb : emergencyManager_.getAllAmbulances()) {
                if (!amb.isAvailable) {
                    emergencyManager_.rerouteActiveAmbulance(amb.id);
                }
            }
            break;
    }
}

void ResponseOptimizer::revertStrategy(const ResponseEvaluation& plan) {
    for (const auto& devId : plan.isolatedDevices) {
        graph_.setDeviceStatus(devId, DeviceOperationalStatus::ONLINE_NORMAL);
    }
    for (uint32_t node : plan.isolatedIntersections) {
        graph_.setIntersectionBlocked(node, false);
    }
    trafficEngine_.updateAdaptiveSignals();
}

} // namespace urbantis
