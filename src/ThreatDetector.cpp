#include "ThreatDetector.hpp"
#include <cmath>
#include <algorithm>
#include <sstream>

namespace urbantis {

// -------------------------------------------------------------
// Attack Signature Trie Implementation
// -------------------------------------------------------------
AttackSignatureTrie::AttackSignatureTrie() : root_(std::make_unique<TrieNode>()) {}

void AttackSignatureTrie::insertSignature(const std::vector<CyberEventType>& sequence,
                                         const std::string& attackName, int severity) {
    TrieNode* curr = root_.get();
    for (CyberEventType et : sequence) {
        int key = static_cast<int>(et);
        if (curr->children.find(key) == curr->children.end()) {
            curr->children[key] = std::make_unique<TrieNode>();
        }
        curr = curr->children[key].get();
    }
    curr->isTerminal = true;
    curr->attackName = attackName;
    curr->severity = severity;
}

bool AttackSignatureTrie::matchSequence(const std::vector<CyberEventType>& sequence,
                                       std::string& outAttackName, int& outSeverity) const {
    // Try matching any suffix of the sequence against the trie
    for (size_t start = 0; start < sequence.size(); ++start) {
        const TrieNode* curr = root_.get();
        for (size_t i = start; i < sequence.size(); ++i) {
            int key = static_cast<int>(sequence[i]);
            auto it = curr->children.find(key);
            if (it == curr->children.end()) {
                break;
            }
            curr = it->second.get();
            if (curr->isTerminal) {
                outAttackName = curr->attackName;
                outSeverity = curr->severity;
                return true;
            }
        }
    }
    return false;
}

// -------------------------------------------------------------
// Streaming Graph Sketch (MIDAS-inspired) Implementation
// -------------------------------------------------------------
StreamingGraphSketch::StreamingGraphSketch(size_t width, size_t depth)
    : width_(width), depth_(depth),
      currentCounts_(depth, std::vector<uint32_t>(width, 0)),
      totalCounts_(depth, std::vector<uint32_t>(width, 0)),
      hashSeeds_({1337, 7331, 9973, 6101}) {}

size_t StreamingGraphSketch::hash(const std::string& key, uint64_t seed) const {
    // 64-bit FNV-1a Hash
    uint64_t h = 14695981039346656037ULL ^ seed;
    for (char c : key) {
        h ^= static_cast<uint64_t>(c);
        h *= 1099511628211ULL;
    }
    return static_cast<size_t>(h % width_);
}

void StreamingGraphSketch::addEdge(const std::string& u, const std::string& v, uint64_t currentTick) {
    if (currentTick != lastTick_) {
        // Reset current tick counts for new time-slice
        for (size_t d = 0; d < depth_; ++d) {
            std::fill(currentCounts_[d].begin(), currentCounts_[d].end(), 0);
        }
        lastTick_ = currentTick;
    }

    std::string edgeKey = u + "->" + v;
    for (size_t d = 0; d < depth_; ++d) {
        size_t bucket = hash(edgeKey, hashSeeds_[d]);
        currentCounts_[d][bucket]++;
        totalCounts_[d][bucket]++;
    }
}

double StreamingGraphSketch::computeAnomalyScore(const std::string& u, const std::string& v, uint64_t currentTick) {
    if (currentTick <= 1) return 0.0;

    std::string edgeKey = u + "->" + v;
    double minScore = std::numeric_limits<double>::max();

    double t = static_cast<double>(currentTick);

    for (size_t d = 0; d < depth_; ++d) {
        size_t bucket = hash(edgeKey, hashSeeds_[d]);
        double cur = static_cast<double>(currentCounts_[d][bucket]);
        double tot = static_cast<double>(totalCounts_[d][bucket]);

        // Expected count in current tick assuming uniform historical distribution: a_hat = tot / t
        double aHat = tot / t;
        if (aHat < 0.1) aHat = 0.1;

        // Chi-squared deviation score
        double variance = aHat * (1.0 - 1.0 / t);
        if (variance < 0.01) variance = 0.01;

        double score = std::pow(cur - aHat, 2) / variance;
        if (score < minScore) {
            minScore = score;
        }
    }
    return minScore;
}

// -------------------------------------------------------------
// ThreatDetector Implementation
// -------------------------------------------------------------
ThreatDetector::ThreatDetector(double anomalyThreshold)
    : anomalyThreshold_(anomalyThreshold) {
    // Pre-populate Attack Killchain Signatures in Trie
    signatureTrie_.insertSignature(
        {CyberEventType::PORT_SCAN, CyberEventType::AUTH_REQUEST, CyberEventType::UNAUTHORIZED_ACCESS, CyberEventType::SIGNAL_OVERRIDE},
        "Traffic Signal Hijack Killchain", 5);

    signatureTrie_.insertSignature(
        {CyberEventType::PORT_SCAN, CyberEventType::PORT_SCAN, CyberEventType::BURST_FLOOD},
        "Reconnaissance-Driven DDoS", 4);

    signatureTrie_.insertSignature(
        {CyberEventType::AUTH_REQUEST, CyberEventType::AUTH_REQUEST, CyberEventType::AUTH_REQUEST, CyberEventType::CONFIG_CHANGE},
        "Credential Brute-Force & Gateway Manipulation", 4);

    signatureTrie_.insertSignature(
        {CyberEventType::MALICIOUS_PAYLOAD, CyberEventType::SIGNAL_OVERRIDE},
        "Direct Malicious Injection Attack", 5);
}

void ThreatDetector::ingestNetworkEvent(const NetworkEvent& event) {
    totalEventsProcessed_++;

    // 1. Streaming dynamic graph microcluster detection
    graphSketch_.addEdge(event.sourceDeviceId, event.targetDeviceId, event.timestamp);
    double score = graphSketch_.computeAnomalyScore(event.sourceDeviceId, event.targetDeviceId, event.timestamp);

    // 2. Sliding window management
    slidingWindowEventWindow_.push_back(event);
    if (slidingWindowEventWindow_.size() > 500) {
        slidingWindowEventWindow_.pop_front();
    }

    // 3. Track per-device sequence history for Killchain analysis
    auto& history = deviceEventHistory_[event.targetDeviceId];
    history.push_back(event.type);
    if (history.size() > 30) {
        history.erase(history.begin(), history.begin() + 10);
    }

    // 4. Anomaly threshold evaluation
    if (score >= anomalyThreshold_) {
        CyberAlert alert;
        alert.alertId = alertIdCounter_++;
        alert.timestamp = event.timestamp;
        alert.anomalyScore = score;
        alert.confidence = std::min(1.0, 0.5 + (score / (anomalyThreshold_ * 4.0)));
        alert.severityLevel = (score > anomalyThreshold_ * 3.0) ? 5 : (score > anomalyThreshold_ * 1.5 ? 4 : 3);
        alert.sourceDeviceId = event.sourceDeviceId;
        alert.targetDeviceId = event.targetDeviceId;
        alert.category = "Microcluster Edge Burst Anomaly";

        std::ostringstream oss;
        oss << "Volumetric burst anomaly detected on edge (" << event.sourceDeviceId << " -> " 
            << event.targetDeviceId << "). Chi-Square score: " << score << " exceeds baseline threshold.";
        alert.explanation = oss.str();

        std::ostringstream ev;
        ev << "[T=" << event.timestamp << "] Event: " << eventTypeToString(event.type)
           << ", Packets: " << event.packetCount << ", Status: " << event.statusCode;
        alert.evidenceChain = ev.str();

        activeAlerts_.push_back(alert);
    }

    // 5. Killchain sequence signature matching via Trie
    std::string attackName;
    int trieSeverity = 1;
    if (signatureTrie_.matchSequence(history, attackName, trieSeverity)) {
        CyberAlert alert;
        alert.alertId = alertIdCounter_++;
        alert.timestamp = event.timestamp;
        alert.anomalyScore = 50.0 + (trieSeverity * 10.0);
        alert.confidence = 0.95;
        alert.severityLevel = trieSeverity;
        alert.sourceDeviceId = event.sourceDeviceId;
        alert.targetDeviceId = event.targetDeviceId;
        alert.category = "Killchain Exploit: " + attackName;

        std::ostringstream oss;
        oss << "Multi-stage attack killchain identified targetting device " << event.targetDeviceId
            << ". Staged sequence matched attack signature: " << attackName;
        alert.explanation = oss.str();

        std::ostringstream ev;
        ev << "Sequence matched in history log: ";
        for (CyberEventType t : history) {
            ev << eventTypeToString(t) << " -> ";
        }
        alert.evidenceChain = ev.str();

        activeAlerts_.push_back(alert);
        history.clear(); // Clear to prevent duplicate fire on same sequence
    }
}

std::vector<CyberAlert> ThreatDetector::getActiveAlerts() const {
    std::vector<CyberAlert> result;
    for (const auto& a : activeAlerts_) {
        if (a.active) result.push_back(a);
    }
    return result;
}

std::vector<CyberAlert> ThreatDetector::getRankedAlerts() const {
    std::vector<CyberAlert> result = getActiveAlerts();
    std::sort(result.begin(), result.end(),
        [](const CyberAlert& a, const CyberAlert& b) {
            if (a.severityLevel != b.severityLevel) {
                return a.severityLevel > b.severityLevel;
            }
            return a.anomalyScore > b.anomalyScore;
        });
    return result;
}

void ThreatDetector::acknowledgeAlert(uint64_t alertId) {
    for (auto& a : activeAlerts_) {
        if (a.alertId == alertId) {
            a.active = false;
        }
    }
}

double ThreatDetector::getCurrentEventRatePerSec() const {
    if (slidingWindowEventWindow_.size() < 2) return 0.0;
    uint64_t firstT = slidingWindowEventWindow_.front().timestamp;
    uint64_t lastT = slidingWindowEventWindow_.back().timestamp;
    double delta = static_cast<double>(lastT - firstT);
    if (delta <= 0.0) return static_cast<double>(slidingWindowEventWindow_.size());
    return static_cast<double>(slidingWindowEventWindow_.size()) / delta;
}

std::vector<int> ThreatDetector::computeKMPPrefixTable(const std::vector<CyberEventType>& pattern) const {
    size_t m = pattern.size();
    std::vector<int> pi(m, 0);
    int k = 0;
    for (size_t q = 1; q < m; ++q) {
        while (k > 0 && pattern[k] != pattern[q]) {
            k = pi[k - 1];
        }
        if (pattern[k] == pattern[q]) {
            k++;
        }
        pi[q] = k;
    }
    return pi;
}

bool ThreatDetector::searchDeviceEventHistoryKMP(const std::string& deviceId, const std::vector<CyberEventType>& pattern) const {
    auto it = deviceEventHistory_.find(deviceId);
    if (it == deviceEventHistory_.end() || pattern.empty()) return false;

    const auto& text = it->second;
    size_t n = text.size();
    size_t m = pattern.size();
    if (n < m) return false;

    std::vector<int> pi = computeKMPPrefixTable(pattern);
    int q = 0;
    for (size_t i = 0; i < n; ++i) {
        while (q > 0 && pattern[q] != text[i]) {
            q = pi[q - 1];
        }
        if (pattern[q] == text[i]) {
            q++;
        }
        if (static_cast<size_t>(q) == m) {
            return true; // Match found
        }
    }
    return false;
}

std::vector<NetworkEvent> ThreatDetector::generateNormalTrafficStream(uint64_t tick, size_t eventCount) {
    std::vector<NetworkEvent> events;
    for (size_t i = 0; i < eventCount; ++i) {
        NetworkEvent ev;
        ev.timestamp = tick;
        ev.sourceDeviceId = "GW-NORTH-01";
        ev.targetDeviceId = "SIG-CTRL-" + std::to_string((i % 15) + 1);
        ev.type = (i % 5 == 0) ? CyberEventType::TELEMETRY_FLOW : CyberEventType::HEARTBEAT;
        ev.packetCount = 1 + (i % 3);
        ev.byteCount = 64 + (i * 16);
        ev.statusCode = 200;
        events.push_back(ev);
    }
    return events;
}

std::vector<NetworkEvent> ThreatDetector::generateDDoSFloodStream(uint64_t tick, const std::string& targetGateway, size_t burstSize) {
    std::vector<NetworkEvent> events;
    for (size_t i = 0; i < burstSize; ++i) {
        NetworkEvent ev;
        ev.timestamp = tick;
        ev.sourceDeviceId = "BOTNET-NODE-" + std::to_string((i % 50) + 100);
        ev.targetDeviceId = targetGateway;
        ev.type = CyberEventType::BURST_FLOOD;
        ev.packetCount = 500;
        ev.byteCount = 64000;
        ev.statusCode = 503;
        ev.payloadSignature = "SYN_FLOOD_BURST";
        events.push_back(ev);
    }
    return events;
}

std::vector<NetworkEvent> ThreatDetector::generateKillchainExploitStream(uint64_t tick, const std::string& attackerIp, const std::string& targetController) {
    std::vector<NetworkEvent> events;

    NetworkEvent e1;
    e1.timestamp = tick;
    e1.sourceDeviceId = attackerIp;
    e1.targetDeviceId = targetController;
    e1.type = CyberEventType::PORT_SCAN;
    e1.payloadSignature = "SYN_SCAN_PORT_8080";
    events.push_back(e1);

    NetworkEvent e2;
    e2.timestamp = tick + 1;
    e2.sourceDeviceId = attackerIp;
    e2.targetDeviceId = targetController;
    e2.type = CyberEventType::AUTH_REQUEST;
    e2.payloadSignature = "BRUTE_FORCE_ADMIN";
    e2.statusCode = 401;
    events.push_back(e2);

    NetworkEvent e3;
    e3.timestamp = tick + 2;
    e3.sourceDeviceId = attackerIp;
    e3.targetDeviceId = targetController;
    e3.type = CyberEventType::UNAUTHORIZED_ACCESS;
    e3.payloadSignature = "CVE_BYPASS_TOKEN";
    e3.statusCode = 200;
    events.push_back(e3);

    NetworkEvent e4;
    e4.timestamp = tick + 3;
    e4.sourceDeviceId = attackerIp;
    e4.targetDeviceId = targetController;
    e4.type = CyberEventType::SIGNAL_OVERRIDE;
    e4.payloadSignature = "SET_GREEN_WAVE_ALL_PHASES";
    e4.statusCode = 200;
    events.push_back(e4);

    return events;
}

} // namespace urbantis
