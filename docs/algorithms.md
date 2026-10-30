# URBANTIS Data Structures and Algorithms (DSA) Manual

This document details the core data structures and algorithmic implementations powering URBANTIS.

---

## 1. Algorithmic Overview & Complexities

| Algorithm / Data Structure | Subsystem | Time Complexity | Space Complexity | Purpose |
| :--- | :--- | :--- | :--- | :--- |
| **Tarjan's Articulation Points** | `CityGraph` | $\mathcal{O}(V + E)$ | $\mathcal{O}(V)$ | Detects single points of failure in the road network |
| **Tarjan's Bridge Decomposition** | `CityGraph` | $\mathcal{O}(V + E)$ | $\mathcal{O}(V)$ | Identifies critical bridges whose severance disconnects sectors |
| **Binary-Heap Dijkstra** | `TrafficEngine` | $\mathcal{O}((V + E) \log V)$ | $\mathcal{O}(V)$ | Baseline dynamic shortest path calculation |
| **Consistent A\* Search** | `TrafficEngine` | $\mathcal{O}((V + E) \log V)$ | $\mathcal{O}(V)$ | Directed shortest path with admissible travel-time lower bound |
| **K-Alternative Diverse Routing** | `TrafficEngine` | $\mathcal{O}(K \cdot (V + E) \log V)$ | $\mathcal{O}(V)$ | Generates disjoint alternative corridors around traffic jams |
| **Bipartite Matching** | `ServiceAssignment` | $\mathcal{O}(R \cdot Q)$ | $\mathcal{O}(R \cdot Q)$ | Matches available riders $R$ to pending requests $Q$ |
| **2-Opt Local Search TSP** | `ServiceAssignment` | $\mathcal{O}(N^2)$ per pass | $\mathcal{O}(N)$ | Optimizes courier stop sequencing to avoid deadline violations |
| **Streaming Graph Sketch** | `ThreatDetector` | $\mathcal{O}(1)$ | $\mathcal{O}(D \cdot W) = \mathcal{O}(1)$ | Real-time edge burst anomaly detection on dynamic streams |
| **Attack Signature Trie** | `ThreatDetector` | $\mathcal{O}(L)$ | $\mathcal{O}(\Sigma \cdot L)$ | Matches multi-stage cyber kill-chains in event history |
| **Knuth-Morris-Pratt (KMP)** | `ThreatDetector` | $\mathcal{O}(N + M)$ | $\mathcal{O}(M)$ | Fast signature substring matching in device audit logs |

---

## 2. Mathematical Formulations

### Dynamic Road Latency: Bureau of Public Roads (BPR) Model
$$\tau(e) = \tau_0(e) \cdot \left[1 + \alpha \left(\frac{V_e}{C_e}\right)^\beta\right] + \Delta_{\text{incident}}$$
Where:
- $\tau_0(e) = \frac{\text{Length}_e}{\text{SpeedLimit}_e}$ (Free-flow travel time)
- $V_e$ is dynamic vehicular volume
- $C_e$ is nominal capacity in vehicles per hour
- Standard coefficients $\alpha = 0.15$, $\beta = 4.0$
- $\Delta_{\text{incident}}$ is synthesized delay caused by crashes or compromised signals.

### A\* Admissible Heuristic
$$h(u, \text{goal}) = \frac{\text{EuclideanDistance}(u, \text{goal})}{\text{MaxPermissibleVelocity}}$$
Because $\text{MaxPermissibleVelocity} \ge \text{ActualVelocity}(e)$ on every edge, $h(u)$ is guaranteed to never overestimate the remaining travel time, preserving strict A\* optimality while visiting 39%–70% fewer vertices than Dijkstra.

### Webster's Optimal Cycle Time Model
$$C_{\text{opt}} = \frac{1.5 L + 5}{1 - Y}$$
Where:
- $L$ is total lost time per cycle (change intervals, default 8.0s)
- $Y = \sum_{i} y_i$ is the sum of critical flow ratios $y_i = \frac{q_i}{s_i}$ (volume / saturation flow)
- Effective green time is split proportionally to directional demand:
  $$g_i = (C_{\text{opt}} - L) \cdot \frac{y_i}{Y}$$

### Streaming Edge Anomaly Detection (MIDAS Engine)
For each directed communication $(u, v)$ observed at tick $t$:
$$\hat{a}_{u,v} = \frac{s_{u,v}}{t}$$
$$\chi^2 = \frac{\left(a_{u,v} - \hat{a}_{u,v}\right)^2}{\max\left(0.01, \hat{a}_{u,v} \cdot (1 - 1/t)\right)}$$
The minimum Chi-squared score across all $D$ independent hash functions provides an unbiased test statistic for sudden microcluster edge bursts.
