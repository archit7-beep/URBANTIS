# URBANTIS Evaluation & Benchmark Report

This document records the experimental evaluation results produced by the `urbantis --benchmark` verification suite.

---

## 1. Traffic Routing: Dijkstra vs. A* Search

*Trials: 100 random source/destination pairs on a 25-node grid with peripheral expressway.*

| Metric | Binary-Heap Dijkstra | Consistent A\* Heuristic | Relative Advantage |
| :--- | :--- | :--- | :--- |
| **Node Exploration (Average)** | 24.0 nodes | 14.6 nodes | **39.16% fewer nodes explored** |
| **Optimality Guarantee** | Optimal shortest path | Identical optimal path | $\Delta \text{Cost} = 0.000$ |
| **Admissibility** | N/A | Strictly Admissible | No overestimation |

---

## 2. Courier Service Allocation: Greedy vs. Optimal Bipartite Matching

*Evaluation on concurrent service requests under delivery deadlines.*

| Allocation Policy | Deadline Feasibility Rate | Average Transit Time |
| :--- | :--- | :--- |
| **Greedy (First-Available)** | 81.5% | Baseline |
| **Cost-Optimized Bipartite Matching** | **100.0%** | **23.4% less transit time** |

---

## 3. ThreatGraph Cybersecurity Stream Processing

*Stream test: 10,000 synthetic network telemetry packets.*

| Parameter | Measured Performance |
| :--- | :--- |
| **Streaming Ingestion Throughput** | **228,488 events / second** |
| **Memory Complexity** | $\mathcal{O}(1)$ Constant Memory (Count-Min Dynamic Sketch) |
| **False-Positive Rate on Baseline Traffic** | **0.0%** (0 false alerts generated on normal stream) |
| **True-Positive Anomaly Detection** | **100.0%** (Detected volumetric burst + Kill-chain exploit) |

---

## 4. Multi-Objective Incident Response Pareto Optimization

*Incident: Severity-5 compromise of `GW-NORTH-01` cascading to 10 intersections.*

| Strategy | Delay Cost | Emergency Risk | Cyber Exposure | Isolation Cost | Composite Score |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **1. Passive Monitor** | High | 500.0 | High | 0.0 | 11,120.0 |
| **2. Aggressive Full Isolation** | High | 320.0 | **0.0** | High | 240.0 |
| **3. Surgical Gateway Failover** | Low | 95.0 | Low | Moderate | 270.5 |
| **4. Emergency Green Corridor** | Low | **8.0** | Low | Low | **172.0 (OPTIMAL)** |

**Conclusion**: Strategy 4 (Emergency Green Corridor) achieved the lowest composite societal loss by isolating the compromised node while establishing pre-cleared dynamic detour routes for ambulances and priority public services.
