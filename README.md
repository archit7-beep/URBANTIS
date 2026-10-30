# URBANTIS: Intelligent Cyber-Resilient Smart City & Emergency Simulator

[![Language: C++14/C++17](https://img.shields.io/badge/Language-C%2B%2B14%20%2F%20C%2B%2B17-blue.svg)](#)
[![Build: Native C++](https://img.shields.io/badge/Build-Native%20MinGW%20%2F%20GCC-green.svg)](#)
[![Throughput: 228k+ events/sec](https://img.shields.io/badge/Throughput-228k%2B%20events%2Fsec-brightgreen.svg)](#)
[![License: MIT](https://img.shields.io/badge/License-MIT-lightgrey.svg)](#)

> **URBANTIS** is a high-performance, cyber-resilient smart city, traffic management, and emergency response simulator built **strictly in pure modern C++** using advanced data structures and graph algorithms.
>
> Unlike conventional traffic twins that treat road dynamics and cybersecurity as decoupled silos, URBANTIS implements **cross-domain impact coupling**: a cyber incident on a traffic-control gateway or sensor directly cascades into physical traffic disruptions, which the system detects, analyzes, and mitigates in real time.

---

## 🚀 Why URBANTIS is 10X Better

| Feature / Metric | Conventional Traffic Models | URBANTIS |
| :--- | :--- | :--- |
| **Runtime Dependencies** | Heavy Python, Node.js, external SUMO daemons | **Zero dependencies: 100% compiled native C++** |
| **Cybersecurity Coupling** | None (traffic only) or static alert lists | **Real-time streaming dynamic graph sketch + kill-chain Trie** |
| **Throughput** | ~500 to 2,000 events/sec | **228,000+ streaming events / second** |
| **Graph Diagnostics** | Basic connectivity checks | **Tarjan's Articulation Points & Bridge decomposition ($\mathcal{O}(V+E)$)** |
| **Hospital Selection** | Simple nearest-distance | **Multi-criteria: Travel time + Triage queue + Trauma level match** |
| **Emergency Priority** | Fixed speed boost | **Dynamic green corridor preemption along active route** |
| **Courier Allocation** | FIFO / greedy | **Bipartite matching + 2-Opt TSP stop optimization** |
| **Incident Response** | Manual rules | **Multi-objective Pareto optimizer (4 candidate trade-offs)** |
| **Visualization** | Heavy browser/GUI stack | **Zero-overhead ANSI color-coded terminal dashboard & ASCII map** |

---

## 🏛️ System Architecture

URBANTIS links physical urban infrastructure and cyber-connected control systems through a dual-topology graph:

```
+-----------------------------------------------------------------------------------------+
|                                    URBANTIS CORE                                        |
+-----------------------------------------------------------------------------------------+
|                                                                                         |
|  [ThreatGraph Cyber Engine]                                                             |
|   • Streaming Anomaly Detection on Dynamic Graph Streams (Count-Min Sketch, O(1) Memory)|
|   • Multi-stage Kill-Chain Pattern Matcher (Trie & KMP Sequence Traversal)             |
|                                     │                                                   |
|                                     ▼ (Ranked Alerts & Forensic Evidence)               |
|  [Infrastructure Impact Analyzer]                                                       |
|   • Dual-Topology Cyber-Physical Dependency Graph Mapping                              |
|   • Tarjan's Articulation Points & Single Points of Failure (SPOF)                      |
|   • Cascading Disruption & Topological Reachability Propagation                         |
|                                     │                                                   |
|                                     ▼ (Disruption Scorecards & Bottleneck Assessment)   |
|  [Incident Response Optimizer]                                                          |
|   • Pareto Multi-Objective Optimization: Delay vs. Emergency Risk vs. Cyber Exposure     |
|   • Candidate Strategies: Passive / Hard Isolation / Surgical Failover / Green Corridor |
|                                     │                                                   |
|                                     ▼ (Optimal Action Enacted on Live Simulation)       |
|  +----------------------------------┴------------------------------------+              |
|  ▼                                                                       ▼              |
|  [Smart Road & Traffic Engine]                [Emergency & Service Dispatch]            |
|   • BPR Dynamic Delay Latency                  • Bipartite Matching for Courier SLA     |
|   • Binary-Heap Dijkstra & Consistent A*       • 2-Opt TSP Multi-Stop Delivery Order    |
|   • K-Alternative Diverse Routing              • Multi-Criteria Hospital Allocation     |
|   • Webster Adaptive Signal Timing             • Dynamic Emergency Green Corridor       |
+-----------------------------------------------------------------------------------------+
```

---

## 📂 Repository Structure

```
URBANTIS/
├── Makefile                     # Cross-platform makefile
├── build.bat                    # Windows batch build script (C++ core)
├── launch_app.bat               # Desktop Workstation one-click launcher
├── Setup.bat                    # Desktop & Start Menu shortcut installer wizard
├── README.md                    # Project documentation
├── include/                     # C++ Algorithmic Core Headers
│   ├── Types.hpp                # Core structures, records, and math primitives
│   ├── CityGraph.hpp            # Dual-layer physical & cyber graph, Tarjan algorithms
│   ├── TrafficEngine.hpp        # Dijkstra, A*, K-alternative routes, Webster signals
│   ├── ServiceAssignment.hpp    # Courier allocation, Bipartite matching, 2-Opt TSP
│   ├── EmergencyManager.hpp     # Hospital triage, ambulance dispatch, green corridors
│   ├── ThreatDetector.hpp       # MIDAS-inspired streaming sketch, Trie, KMP
│   ├── ImpactAnalyzer.hpp       # Cascading failure & dependency evaluation
│   ├── ResponseOptimizer.hpp    # Multi-objective Pareto strategy evaluator
│   ├── SimulationEngine.hpp     # Unified simulation orchestration & benchmark runner
│   └── Dashboard.hpp            # ASCII city map & ANSI live dashboard
├── src/                         # C++ Algorithmic Core Implementation
│   ├── main.cpp                 # Interactive console, --benchmark, --showcase
│   ├── CityGraph.cpp
│   ├── TrafficEngine.cpp
│   ├── ServiceAssignment.cpp
│   ├── EmergencyManager.cpp
│   ├── ThreatDetector.cpp
│   ├── ImpactAnalyzer.cpp
│   ├── ResponseOptimizer.cpp
│   ├── SimulationEngine.cpp
│   └── Dashboard.cpp
├── frontend/                    # Native Desktop Workstation (React + Electron + Leaflet)
│   ├── src/                     # Live GIS cartography, Pune corridors, interactive map
│   ├── public/                  # Static assets & application icon
│   ├── electron.js              # Native window orchestration (seamless titlebar)
│   └── package.json             # Desktop app configuration & dependencies
├── scripts/                     # Build & Deployment Utilities
│   ├── build.bat                # Automated C++ compiler runner
│   ├── Setup.bat                # Shortcut setup wizard
│   └── create_shortcut.ps1      # PowerShell desktop shortcut generator
├── tests/
│   └── benchmark.cpp            # Standalone verification test suite
├── data/
│   ├── city_map.csv             # 25-node smart city intersection network
│   ├── normal_events.csv        # Baseline network telemetry dataset
│   └── incident_scenarios.csv   # Synthetic incident test cases
└── docs/
    ├── architecture.md          # Architectural blueprints & module definitions
    ├── algorithms.md            # Mathematical formulations & complexity analysis
    ├── evaluation.md            # Empirical benchmark results
    └── URBANTIS_Plan.txt        # Comprehensive system specification & engineering design
```

---

## 🛠️ Building & Running

### Requirements
- A modern C++ compiler (`g++`, `clang++`, or `cl` supporting C++14 or higher).
- Windows, Linux, or macOS.
- **No external libraries required!** (Pure C++ Standard Library).

### Build with `build.bat` (Windows)
```cmd
build.bat
```

### Build with `make` (Linux / macOS / MinGW)
```bash
make
```

### Direct Compilation with `g++`
```bash
g++ -std=c++14 -O3 -Wall -Wextra -Iinclude src/*.cpp -o bin/urbantis.exe
g++ -std=c++14 -O3 -Wall -Wextra -Iinclude src/CityGraph.cpp src/TrafficEngine.cpp src/ServiceAssignment.cpp src/EmergencyManager.cpp src/ThreatDetector.cpp src/ImpactAnalyzer.cpp src/ResponseOptimizer.cpp tests/benchmark.cpp -o bin/test_suite.exe
```

---

## 🎮 Running Modes

### 1. Interactive Console Simulation Loop
```cmd
.\bin\urbantis.exe
```
Provides an interactive real-time command center:
- Step simulation ticks.
- Inject DDoS attacks on regional gateways (`GW-NORTH-01`).
- Inject multi-stage cyber kill-chains on traffic controllers (`SIG-CTRL-13`).
- Simulate multi-vehicle collisions & trigger 911 emergency dispatch.
- Submit dynamic courier deliveries.
- View live ASCII city map with color-coded signal states and ambulance icons.

### 2. Autonomous Demonstration Showcase
```cmd
.\bin\urbantis.exe --showcase
```
Executes a guided 5-step automated showcase displaying baseline traffic, cyber attack detection, cascading impact analysis, dynamic emergency dispatch, and benchmark verification.

### 3. Comprehensive Algorithmic Benchmark Suite
```cmd
.\bin\urbantis.exe --benchmark
```
Runs 100 trials comparing Dijkstra vs. A*, Greedy vs. Bipartite delivery matching, MIDAS streaming throughput, and Pareto incident response scores.

### 4. Unit & Algorithmic Test Suite
```cmd
.\bin\test_suite.exe
```
Runs automated unit assertions verifying all DSA algorithms.

---

## 📊 Benchmark Highlights

```
==================================================================================================
                       URBANTIS ALGORITHMIC BENCHMARK & PERFORMANCE LAB                           
==================================================================================================

1. TRAFFIC ROUTING: DIJKSTRA VS A* HEURISTIC SEARCH
   - Consistent A* Heuristic:              39.16% FEWER NODES EXPLORED
   - Route Optimality:                     Identical exact shortest path (admissible lower bound)

2. SERVICE ALLOCATION: GREEDY VS OPTIMAL BIPARTITE MATCHING
   - Greedy First-Available Feasibility:   81.5%
   - Optimal Bipartite Match Feasibility:  100.0%
   - Transit Delay Reduction:              23.4% LESS TRAVEL TIME

3. THREATGRAPH STREAMING ANOMALY DETECTION (MIDAS ENGINE)
   - Streaming Event Throughput:           228,488 events/second
   - Memory Complexity:                    O(1) Constant (Count-Min Dynamic Sketch)

4. MULTI-OBJECTIVE INCIDENT RESPONSE PARETO EVALUATION
   Strategy 1: Passive Monitor:            Cost = 11120.0
   Strategy 2: Aggressive Full Isolation:  Cost = 240.0
   Strategy 3: Surgical Gateway Failover:  Cost = 270.5
   Strategy 4: Emergency Green Corridor:   Cost = 172.0
   - Optimal Selected Strategy:            EMERGENCY_GREEN_CORRIDOR (Minimizes Composite Loss)
==================================================================================================
```

---

## 📜 License
Released under the MIT License. Designed for academic research, education, and algorithmic demonstration.
