#pragma once

#include "Types.hpp"
#include <vector>
#include <deque>
#include <unordered_map>
#include <string>
#include <memory>

namespace urbantis {

// Node for Kill-Chain Signature Trie
struct TrieNode {
    std::unordered_map<int, std::unique_ptr<TrieNode>> children;
    bool isTerminal{false};
    std::string attackName;
    int severity{1};
};

// Attack Signature Trie for multi-event detection
class AttackSignatureTrie {
public:
    AttackSignatureTrie();
    void insertSignature(const std::vector<CyberEventType>& sequence, const std::string& attackName, int severity);
    bool matchSequence(const std::vector<CyberEventType>& sequence, std::string& outAttackName, int& outSeverity) const;

private:
    std::unique_ptr<TrieNode> root_;
};

// Count-Min Sketch / Hash Table for Streaming Edge Anomaly Detection (MIDAS-inspired)
class StreamingGraphSketch {
public:
    StreamingGraphSketch(size_t width = 2048, size_t depth = 4);
    void addEdge(const std::string& u, const std::string& v, uint64_t currentTick);
    double computeAnomalyScore(const std::string& u, const std::string& v, uint64_t currentTick);

private:
    size_t width_;
    size_t depth_;
    std::vector<std::vector<uint32_t>> currentCounts_; // Counts in current tick
    std::vector<std::vector<uint32_t>> totalCounts_;   // Cumulative historical counts
    std::vector<uint64_t> hashSeeds_;
    uint64_t lastTick_{0};

    size_t hash(const std::string& key, uint64_t seed) const;
};

class ThreatDetector {
public:
    explicit ThreatDetector(double anomalyThreshold = 12.0);

    // Event Ingestion & Streaming Analysis
    void ingestNetworkEvent(const NetworkEvent& event);

    // Explainable Alert Processing
    std::vector<CyberAlert> getActiveAlerts() const;
    std::vector<CyberAlert> getRankedAlerts() const;
    void acknowledgeAlert(uint64_t alertId);

    // Sliding Window Statistics
    size_t getEventsProcessedCount() const { return totalEventsProcessed_; }
    double getCurrentEventRatePerSec() const;

    // KMP Substring / Signature Search over Device Event Logs
    bool searchDeviceEventHistoryKMP(const std::string& deviceId, const std::vector<CyberEventType>& pattern) const;

    // Synthetic Cyber Incident Generators for reproducible experiments
    std::vector<NetworkEvent> generateNormalTrafficStream(uint64_t tick, size_t eventCount);
    std::vector<NetworkEvent> generateDDoSFloodStream(uint64_t tick, const std::string& targetGateway, size_t burstSize);
    std::vector<NetworkEvent> generateKillchainExploitStream(uint64_t tick, const std::string& attackerIp, const std::string& targetController);

private:
    double anomalyThreshold_;
    uint64_t alertIdCounter_{1};
    size_t totalEventsProcessed_{0};

    StreamingGraphSketch graphSketch_;
    AttackSignatureTrie signatureTrie_;

    // Sliding window of recent events (last 10 seconds / 500 events)
    std::deque<NetworkEvent> slidingWindowEventWindow_;
    
    // Per-device history for sequence analysis
    std::unordered_map<std::string, std::vector<CyberEventType>> deviceEventHistory_;

    std::vector<CyberAlert> activeAlerts_;

    // KMP Table Precomputation helper
    std::vector<int> computeKMPPrefixTable(const std::vector<CyberEventType>& pattern) const;
};

} // namespace urbantis
