# URBANTIS Architecture Specification

## 1. Overview
URBANTIS is a unified C++ cyber-resilient smart city, dynamic traffic, and emergency management simulation platform. It models urban infrastructure as a **dual-layer cyber-physical graph**:

1. **Physical Transportation Layer**: Intersections (vertices) and road segments (directed edges) governed by dynamic congestion, traffic signals, and vehicle flow.
2. **Cyber-Connected Control Layer**: Traffic signal controllers, regional gateways, and municipal servers communicating via streaming network events.

---

## 2. High-Level Modular Design

```
+----------------------------------------------------------------------------------------------------+
|                                         URBANTIS CORE                                              |
+----------------------------------------------------------------------------------------------------+
|                                                                                                    |
|  [Module D: ThreatGraph Cyber Engine]                                                              |
|   - Dynamic Edge-Stream Microcluster Anomaly Detection (Count-Min Dynamic Sketch, O(1) Memory)     |
|   - Multi-Stage Attack Kill-Chain Matcher (Trie & KMP Sequence Traversal)                         |
|                                     │                                                              |
|                                     ▼ (Ranked Alerts & Forensic Evidence)                          |
|  [Module E: Infrastructure Impact & Dependency Analyzer]                                           |
|   - Cyber-Physical Dual-Topology Dependency Graph Mapping                                          |
|   - Tarjan's Articulation Points & Structural Bottleneck Identification                            |
|   - Cascading Disruption & Network Reachability Propagation                                        |
|                                     │                                                              |
|                                     ▼ (Impact Assessments & Sector Disruption Scores)              |
|  [Module F: Incident Response Optimizer]                                                           |
|   - Multi-Objective Pareto Cost Evaluation (Delay vs. Emergency Risk vs. Cyber Exposure)          |
|   - Candidate Evaluation: Passive Monitor / Full Isolation / Surgical Failover / Green Corridor    |
|                                     │                                                              |
|                                     ▼ (Optimal Action Enacted On Live Simulation)                  |
|  +----------------------------------┴------------------------------------+                         |
|  ▼                                                                       ▼                         |
|  [Module A: Smart Road & Traffic Engine]      [Module B & C: Service & Emergency Dispatch]         |
|   - Dynamic BPR Delay Models                   - Bipartite Matching for Courier SLA                |
|   - Binary-Heap Dijkstra & Consistent A*       - 2-Opt TSP Multi-Stop Delivery Sequencing          |
|   - K-Alternative Diverse Corridors            - Multi-Criteria Hospital Triage Selection          |
|   - Adaptive Webster Signal Timing             - Dynamic Emergency Preemption Green Wave           |
+----------------------------------------------------------------------------------------------------+
```

---

## 3. Subsystem Descriptions

### Module A: Smart Road & Traffic Management
- Models physical streets with lengths, speed limits, lane capacities, and vehicle flows.
- Implements the Bureau of Public Roads (BPR) latency model:
  $$T = T_0 \cdot \left(1 + 0.15 \cdot \left(\frac{V}{C}\right)^4\right) + \text{Delay}_{\text{incident}}$$
- Webster's equisaturation cycle length and adaptive green split calculations.

### Module B & C: Public Service & Emergency Management
- Minimum cost Bipartite Matching for urban parcel and medical deliveries.
- 2-Opt local search TSP for couriers handling multiple drops.
- Multi-objective hospital triage allocation factoring travel time, available beds, trauma care tier, and emergency queue delays.
- Dynamic green wave corridor preemption along ambulance trajectory.

### Module D: ThreatGraph Cybersecurity Engine
- Streaming graph anomaly detection tracking frequency deviations in edge streams using dynamic 2D sketch matrices in $\mathcal{O}(1)$ update time.
- Attack signature Trie identifying multi-event attack patterns (Port Scan $\to$ Brute-force Login $\to$ Privilege Escalation $\to$ Signal Override).
- Knuth-Morris-Pratt (KMP) linear-time log sequence verification.

### Module E: Infrastructure Impact Analyzer
- Bidirectional mapping linking cyber device identifiers (`GW-NORTH-01`, `SIG-CTRL-13`) to physical intersection clusters.
- Tarjan's bridge and articulation point decomposition identifying vulnerable topological bottlenecks.

### Module F: Incident Response Optimizer
- Cost function:
  $$J = w_1 \cdot \text{TrafficDelay} + w_2 \cdot \text{EmergencyRisk} + w_3 \cdot \text{CyberExposure} + w_4 \cdot \text{IsolationPenalty}$$
- Compares and executes the strategy minimizing total societal loss.
