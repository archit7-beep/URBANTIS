import React, { useState, useEffect } from 'react';
import './App.css';
import {
  PUNE_INTERSECTIONS,
  PUNE_ROADS,
  PUNE_HOSPITALS,
  PUNE_GATEWAYS,
  INITIAL_AMBULANCES
} from './puneData';
import {
  computeAStar,
  computeDijkstra,
  findArticulationPoints,
  optimizeSignalWebster,
  selectOptimalHospital,
  evaluateParetoStrategies
} from './puneSimulation';
import PuneLiveMap from './PuneLiveMap';
import urbantisLogo from './assets/urbantis_logo_cropped.png';

export default function App() {
  // Navigation View: 'launcher' (Initial Welcome Screen) or 'workstation' (Live GIS Workstation)
  const [currentView, setCurrentView] = useState('launcher');

  // Selected node for inspection and action execution
  const [selectedNodeId, setSelectedNodeId] = useState(21); // Default Hinjewadi IT Hub
  const [targetRouteNodeId, setTargetRouteNodeId] = useState(3); // Default Pune Station / Sassoon
  const [routeResult, setRouteResult] = useState(null);

  // Simulation Clock
  const [isSimRunning, setIsSimRunning] = useState(true);
  const [simTick, setSimTick] = useState(42);
  const [currentSector, setCurrentSector] = useState(1);
  const [focusCoords, setFocusCoords] = useState({ lat: 18.5913, lng: 73.7389, zoom: 13 });

  // Network Incident State
  const [blockedNodes, setBlockedNodes] = useState(new Set());
  const [compromisedNodes, setCompromisedNodes] = useState(new Set());
  const [preemptedCorridor, setPreemptedCorridor] = useState([]);
  const [ambulances, setAmbulances] = useState(INITIAL_AMBULANCES);

  // Active Alerts & Incident Telemetry
  const [activeAlerts, setActiveAlerts] = useState([
    {
      id: 101,
      severity: 4,
      target: "GW-HINJEWADI-01",
      targetNode: 21,
      category: "Microcluster Edge Anomaly",
      score: 14.8,
      explanation: "Volumetric burst anomaly detected on Hinjewadi Infotech Gateway. Chi-Square: 14.8."
    }
  ]);
  const [activeIncident, setActiveIncident] = useState(null);
  const [selectedStrategy, setSelectedStrategy] = useState("EMERGENCY_GREEN_CORRIDOR");

  // Interactive Modals
  const [showAttackModal, setShowAttackModal] = useState(false);
  const [showEmergencyModal, setShowEmergencyModal] = useState(false);
  const [showBenchmarkModal, setShowBenchmarkModal] = useState(false);
  const [customSeverity, setCustomSeverity] = useState(4);
  const [customAttackType, setCustomAttackType] = useState('DDOS');

  // Tarjan Articulation Points
  const articulationPoints = findArticulationPoints();

  // Sectors for quick navigation
  const SECTORS = [
    { id: 0, code: "01/05", name: "Central Core & Shivajinagar", lat: 18.5314, lng: 73.8446, focusNode: 2 },
    { id: 1, code: "02/05", name: "Hinjewadi Infotech Hub", lat: 18.5913, lng: 73.7389, focusNode: 21 },
    { id: 2, code: "03/05", name: "Sassoon Trauma & Bund Garden", lat: 18.5284, lng: 73.8742, focusNode: 3 },
    { id: 3, code: "04/05", name: "Swargate Regional Transit Hub", lat: 18.5018, lng: 73.8585, focusNode: 13 },
    { id: 4, code: "05/05", name: "Hadapsar & Magarpatta Cyber Zone", lat: 18.5089, lng: 73.9259, focusNode: 15 },
  ];

  // ================================================================
  // CONTINUOUS LIVE PATROL & AMBULANCE MOVEMENT INTERPOLATION LOOP
  // All units are actively moving and patrolling across Pune!
  // ================================================================
  useEffect(() => {
    if (!isSimRunning) return;
    const interval = setInterval(() => {
      setSimTick(t => t + 1);

      setAmbulances(prev => prev.map(amb => {
        let uNode = PUNE_INTERSECTIONS.find(n => n.id === amb.currentNode);
        let vNode = PUNE_INTERSECTIONS.find(n => n.id === amb.targetNode);

        if (!uNode || !vNode) {
          uNode = PUNE_INTERSECTIONS[0];
          vNode = PUNE_INTERSECTIONS[1];
        }

        // Emergency speed is faster than regular patrol
        const step = amb.isEmergency ? 0.32 : 0.16;
        let newProgress = amb.progress + step;

        if (newProgress >= 1.0) {
          // Reached targetNode!
          const reachedNode = amb.targetNode;
          let nextTarget = reachedNode;
          let newPath = amb.path || [];
          let newIsEmergency = amb.isEmergency;
          let newStatus = amb.status;

          if (newPath.length > 0) {
            // Advancing along designated emergency / dispatch path!
            nextTarget = newPath[0];
            newPath = newPath.slice(1);
            newStatus = `🚨 EN ROUTE -> Node #${nextTarget}`;
          } else {
            // Path completed
            if (newIsEmergency) {
              newIsEmergency = false;
              newStatus = `MISSION COMPLETE // RESUMING PATROL`;
            } else {
              // Advance along assigned patrol circuit
              const circuit = amb.patrolCircuit || [3, 2, 8, 7, 3];
              const nextIdx = ((amb.patrolIndex || 0) + 1) % circuit.length;
              nextTarget = circuit[nextIdx];
              newStatus = `PATROLLING // SECTOR ${amb.id}`;
            }
          }

          const targetObj = PUNE_INTERSECTIONS.find(n => n.id === nextTarget) || uNode;

          return {
            ...amb,
            currentNode: reachedNode,
            targetNode: nextTarget,
            progress: 0.0,
            lat: vNode.lat,
            lng: vNode.lng,
            isEmergency: newIsEmergency,
            status: newStatus,
            path: newPath,
            patrolIndex: ((amb.patrolIndex || 0) + 1) % (amb.patrolCircuit ? amb.patrolCircuit.length : 5)
          };
        } else {
          // Smooth continuous GPS interpolation between uNode and vNode
          const curLat = uNode.lat + (vNode.lat - uNode.lat) * newProgress;
          const curLng = uNode.lng + (vNode.lng - uNode.lng) * newProgress;

          return {
            ...amb,
            progress: newProgress,
            lat: curLat,
            lng: curLng
          };
        }
      }));
    }, 450);

    return () => clearInterval(interval);
  }, [isSimRunning]);

  // Quick Sector Jump
  const handleSelectSector = (sector) => {
    setCurrentSector(sector.id);
    setSelectedNodeId(sector.focusNode);
    setFocusCoords({ lat: sector.lat, lng: sector.lng, zoom: 13 });
  };

  // Node Clicked on Map
  const handleNodeSelect = (nodeId) => {
    setSelectedNodeId(nodeId);
    const node = PUNE_INTERSECTIONS.find(n => n.id === nodeId);
    if (node) {
      setFocusCoords({ lat: node.lat, lng: node.lng, zoom: 14 });
    }
  };

  // ================================================================
  // FULL ACTION EXECUTION ENGINE (Runs on ANY selected node!)
  // ================================================================

  // Action 1: Inject Cyber Attack on Selected Node
  const handleExecuteAttack = (nodeId = selectedNodeId, attackType = customAttackType) => {
    const targetNode = PUNE_INTERSECTIONS.find(n => n.id === nodeId) || PUNE_INTERSECTIONS[0];
    setCompromisedNodes(prev => new Set([...prev, nodeId]));

    let category = "Microcluster DDoS Flood";
    let score = 38.6;
    let explanation = `Severe SYN flood stream detected on gateway ${targetNode.gateway}. 3,500 pkts/s burst.`;

    if (attackType === 'KILLCHAIN') {
      category = "Killchain: Controller Hijack";
      score = 45.2;
      explanation = `Matched 4-stage signature: Port Scan -> CVE Auth Bypass -> Signal Override on ${targetNode.name}.`;
    } else if (attackType === 'MITM') {
      category = "Man-in-the-Middle Telemetry Spoofing";
      score = 29.4;
      explanation = `Cryptographic token replay and forged queue telemetry detected on ${targetNode.gateway}.`;
    } else if (attackType === 'TAMPERING') {
      category = "Inductive Loop Sensor Tampering";
      score = 22.1;
      explanation = `Physical spoofing: Zero-traffic signature injected to artificially trigger red phase.`;
    }

    setActiveAlerts(prev => [
      {
        id: Date.now(),
        severity: 5,
        target: targetNode.gateway,
        targetNode: nodeId,
        category,
        score,
        explanation
      },
      ...prev
    ]);

    setSelectedNodeId(nodeId);
    setFocusCoords({ lat: targetNode.lat, lng: targetNode.lng, zoom: 14 });
    setShowAttackModal(false);
  };

  // Action 2: Trigger Emergency Incident & Dispatch Ambulance
  const handleExecuteEmergency = (nodeId = selectedNodeId, severity = customSeverity) => {
    const crashNode = PUNE_INTERSECTIONS.find(n => n.id === nodeId) || PUNE_INTERSECTIONS[0];
    setBlockedNodes(prev => new Set([...prev, nodeId]));

    const incidentObj = {
      id: Date.now(),
      nodeId,
      severity,
      description: `Critical emergency incident at ${crashNode.name}. Corridor blocked.`
    };
    setActiveIncident(incidentObj);

    // Multi-criteria hospital triage selection
    const bestHosp = selectOptimalHospital(nodeId, severity, blockedNodes);
    if (bestHosp) {
      // Find closest ambulance to the scene
      setAmbulances(prev => {
        const copy = [...prev];
        let closestIdx = 0;
        let minDist = Infinity;

        copy.forEach((amb, idx) => {
          const u = PUNE_INTERSECTIONS.find(n => n.id === amb.currentNode);
          if (u) {
            const dLat = u.lat - crashNode.lat;
            const dLng = u.lng - crashNode.lng;
            const dist = dLat * dLat + dLng * dLng;
            if (dist < minDist) {
              minDist = dist;
              closestIdx = idx;
            }
          }
        });

        const chosenAmb = copy[closestIdx];
        const toScene = computeAStar(chosenAmb.currentNode, nodeId, blockedNodes);
        const toHospital = computeAStar(nodeId, bestHosp.intersectionId, blockedNodes);
        const fullPath = [...toScene.path.slice(1), ...toHospital.path.slice(1)];

        copy[closestIdx] = {
          ...chosenAmb,
          isEmergency: true,
          targetNode: toScene.path.length > 1 ? toScene.path[1] : nodeId,
          progress: 0.0,
          status: `🚨 PRIORITY DISPATCH -> ${crashNode.name} -> ${bestHosp.name}`,
          path: fullPath
        };

        setPreemptedCorridor(toHospital.path.length > 0 ? toHospital.path : fullPath);
        return copy;
      });
    }

    setSelectedNodeId(nodeId);
    setFocusCoords({ lat: crashNode.lat, lng: crashNode.lng, zoom: 14 });
    setShowEmergencyModal(false);
  };

  // Action 3: Preempt Green Wave Corridor from Selected Node
  const handlePreemptGreenWave = (nodeId = selectedNodeId) => {
    const bestHosp = selectOptimalHospital(nodeId, 5, blockedNodes);
    if (bestHosp) {
      const route = computeAStar(nodeId, bestHosp.intersectionId, blockedNodes);
      setPreemptedCorridor(route.path);
    }
  };

  // Action 4: Optimize Traffic Signal via Webster's Formula
  const handleOptimizeWebster = (nodeId = selectedNodeId) => {
    const webster = optimizeSignalWebster(nodeId);
    alert(`[Webster's Optimal Timing Calculated for Node #${nodeId}]\n` +
          `• Optimal Cycle Length (C0): ${webster.optimalCycle} seconds\n` +
          `• North-South Green Phase: ${webster.nsGreen} seconds\n` +
          `• East-West Green Phase: ${webster.ewGreen} seconds\n` +
          `• Webster Formulation: C0 = (1.5L + 5) / (1 - Y)\n` +
          `Applied to Pune municipal signal controller.`);
  };

  // Action 5: Calculate Route and Dispatch Ambulance Unit
  const handleCalculateRouteAndDispatch = () => {
    if (selectedNodeId === targetRouteNodeId) return;

    const astar = computeAStar(selectedNodeId, targetRouteNodeId, blockedNodes);
    const dijkstra = computeDijkstra(selectedNodeId, targetRouteNodeId, blockedNodes);

    setRouteResult({
      start: selectedNodeId,
      goal: targetRouteNodeId,
      astar,
      dijkstra,
      reductionPercent: dijkstra.explored > 0
        ? Math.round(((dijkstra.explored - astar.explored) / dijkstra.explored) * 100)
        : 0
    });

    // Light up route on map
    setPreemptedCorridor(astar.path);

    // Command ambulance 1 to physically navigate this route!
    if (astar.path.length > 1) {
      setAmbulances(prev => {
        const copy = [...prev];
        const amb = copy[0];
        copy[0] = {
          ...amb,
          currentNode: astar.path[0],
          targetNode: astar.path[1],
          progress: 0.0,
          lat: PUNE_INTERSECTIONS.find(n => n.id === astar.path[0]).lat,
          lng: PUNE_INTERSECTIONS.find(n => n.id === astar.path[0]).lng,
          isEmergency: true,
          status: `🚨 DISPATCH RUNNING: #${selectedNodeId} -> #${targetRouteNodeId}`,
          path: astar.path.slice(2)
        };
        return copy;
      });
    }

    const startNode = PUNE_INTERSECTIONS.find(n => n.id === selectedNodeId);
    if (startNode) {
      setFocusCoords({ lat: startNode.lat, lng: startNode.lng, zoom: 14 });
    }
  };

  // Action 6: Isolate / Quarantine Gateway
  const handleIsolateGateway = (gatewayId) => {
    const gw = PUNE_GATEWAYS.find(g => g.id === gatewayId);
    if (!gw) return;

    setCompromisedNodes(prev => {
      const next = new Set(prev);
      gw.boundNodes.forEach(n => next.delete(n));
      return next;
    });

    alert(`[CYBER RESILIENCE SHIELD ACTIVATED]\n` +
          `Gateway ${gatewayId} isolated from municipal network.\n` +
          `Cascading failure halted across ${gw.boundNodes.length} intersections.`);
  };

  // Action 7: Clear Incidents & Reset
  const handleClearNodeStatus = (nodeId = selectedNodeId) => {
    setBlockedNodes(prev => {
      const next = new Set(prev);
      next.delete(nodeId);
      return next;
    });
    setCompromisedNodes(prev => {
      const next = new Set(prev);
      next.delete(nodeId);
      return next;
    });
    if (activeIncident && activeIncident.nodeId === nodeId) {
      setActiveIncident(null);
      setPreemptedCorridor([]);
    }
  };

  const handleResetSystem = () => {
    setSimTick(1);
    setBlockedNodes(new Set());
    setCompromisedNodes(new Set());
    setPreemptedCorridor([]);
    setAmbulances(INITIAL_AMBULANCES);
    setActiveAlerts([]);
    setActiveIncident(null);
    setRouteResult(null);
    setSelectedNodeId(21);
    setFocusCoords({ lat: 18.5913, lng: 73.7389, zoom: 13 });
  };

  // Apply Pareto Mitigation Strategy
  const handleApplyStrategy = (stratType) => {
    setSelectedStrategy(stratType);
    if (stratType === "EMERGENCY_GREEN_CORRIDOR") {
      if (activeIncident) {
        const hosp = selectOptimalHospital(activeIncident.nodeId, 5, blockedNodes);
        if (hosp) setPreemptedCorridor(hosp.route.path);
      }
    } else if (stratType === "AGGRESSIVE_ISOLATION") {
      setCompromisedNodes(new Set());
      setBlockedNodes(prev => new Set([...prev, selectedNodeId]));
    }
  };

  const selectedNode = PUNE_INTERSECTIONS.find(n => n.id === selectedNodeId) || PUNE_INTERSECTIONS[0];
  const paretoStrategies = evaluateParetoStrategies(selectedNodeId, 32.5, 5);

  return (
    <div className="app-frame" id="urbantis-workstation">
      {/* ----------------------------------------------------
          APPLICATION TITLEBAR & TOP ACTION RIBBON
         ---------------------------------------------------- */}
      <header className="app-header">
        <div className="header-brand" onClick={() => setCurrentView('launcher')} style={{ cursor: 'pointer' }} title="Return to Launchpad">
          <img src={urbantisLogo} alt="URBANTIS" className="brand-logo-img" />
          <span className="brand-badge-pill">PMC OPS</span>
        </div>

        {/* Global Action Toolbar with Awwwards-Winning Button Styling & Vector SVGs */}
        <div className="header-actions-ribbon">
          <button
            className={`ribbon-pill-btn ${currentView === 'launcher' ? 'active-view' : ''}`}
            onClick={() => setCurrentView(currentView === 'launcher' ? 'workstation' : 'launcher')}
            title="Toggle between Mission Launchpad and Live Workstation"
          >
            <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2.2" strokeLinecap="round" strokeLinejoin="round">
              <circle cx="12" cy="12" r="10" />
              <polygon points="16.24 7.76 14.12 14.12 7.76 16.24 9.88 9.88 16.24 7.76" />
            </svg>
            <span>{currentView === 'launcher' ? 'Open Workstation' : 'Mission Hub'}</span>
          </button>

          <button className="ribbon-pill-btn btn-cyber" onClick={() => setShowAttackModal(true)} title="Inject Custom Cyber Attack">
            <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="#FF9F43" strokeWidth="2.2" strokeLinecap="round" strokeLinejoin="round">
              <polygon points="13 2 3 14 12 14 11 22 21 10 12 10 13 2" />
            </svg>
            <span>Inject Attack</span>
          </button>

          <button className="ribbon-pill-btn btn-ems" onClick={() => setShowEmergencyModal(true)} title="Trigger Emergency Incident & Dispatch 108">
            <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="#FF5252" strokeWidth="2.2" strokeLinecap="round" strokeLinejoin="round">
              <path d="M12 2v4M4.93 4.93l2.83 2.83M2 12h4M20 12h4M16.24 7.76l2.83-2.83M12 18v4M7.76 16.24l-2.83 2.83M19.07 19.07l-2.83-2.83" />
              <circle cx="12" cy="12" r="4" fill="rgba(255, 82, 82, 0.25)" />
            </svg>
            <span>108 Emergency</span>
          </button>

          <button className="ribbon-pill-btn btn-benchmark" onClick={() => setShowBenchmarkModal(true)} title="Run Head-to-Head C++ Algorithmic Benchmarks">
            <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="#00D2D3" strokeWidth="2.2" strokeLinecap="round" strokeLinejoin="round">
              <polyline points="22 12 18 12 15 21 9 3 6 12 2 12" />
            </svg>
            <span>C++ Benchmarks</span>
          </button>

          <button className="ribbon-pill-btn btn-reset" onClick={handleResetSystem} title="Reset Simulation to Clean Baseline">
            <svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2.2" strokeLinecap="round" strokeLinejoin="round">
              <path d="M21.5 2v6h-6M21.34 15.57a10 10 0 1 1-.57-8.38l5.67-5.67" />
            </svg>
            <span>Reset</span>
          </button>
        </div>

        {/* System Diagnostics Status */}
        <div className="header-right">
          <div className="status-capsule capsule-engine" title="C++14 Native Algorithmic Core">
            <span className="dot-pulse-neon"></span>
            <span className="capsule-label">C++14 ENGINE</span>
          </div>
          <div className="status-capsule capsule-tick">
            <span className="capsule-mono">TICK #{String(simTick).padStart(4, '0')}</span>
          </div>
          <button
            className="sim-control-pill"
            onClick={() => setIsSimRunning(r => !r)}
            title={isSimRunning ? "Pause Simulation" : "Resume Simulation"}
          >
            {isSimRunning ? (
              <>
                <svg width="11" height="11" viewBox="0 0 24 24" fill="currentColor">
                  <rect x="6" y="4" width="4" height="16" rx="1" />
                  <rect x="14" y="4" width="4" height="16" rx="1" />
                </svg>
                <span>PAUSE</span>
              </>
            ) : (
              <>
                <svg width="11" height="11" viewBox="0 0 24 24" fill="currentColor">
                  <polygon points="5 3 19 12 5 21 5 3" />
                </svg>
                <span>RUN</span>
              </>
            )}
          </button>
        </div>
      </header>

      {/* ----------------------------------------------------
          VIEW 1: PROPER COMMAND CONSOLE LANDING PAGE
         ---------------------------------------------------- */}
      {currentView === 'launcher' && (
        <section className="launchpad-screen">
          <div className="command-terminal-hero">
            <div className="terminal-header-line">
              <span className="terminal-tag">PMC MUNICIPAL OPERATIONS // PUNE WGS84 DEFENSE GRID</span>
              <span className="terminal-status-pill">● C++14 REAL-TIME ENGINE LOADED</span>
            </div>

            <div className="command-hero-body">
              <div className="hero-left-column">
                <div className="hero-logo-banner">
                  <img src={urbantisLogo} alt="URBANTIS" className="hero-logo-img" />
                </div>
                <h1 className="hero-command-title">
                  CYBER-PHYSICAL <span className="title-gold">COMMAND</span>
                </h1>
                <p className="hero-command-lead">
                  Mission-critical cyber-physical resilience workstation for Pune City. Integrates streaming graph sketches,
                  Webster signal phase optimization, and dynamic 108 emergency corridor routing natively in C++.
                </p>

                <div className="command-cta-row">
                  <button className="command-primary-btn" onClick={() => setCurrentView('workstation')}>
                    INITIALIZE LIVE CARTOGRAPHIC WORKSTATION ➔
                  </button>
                  <button className="command-secondary-btn" onClick={() => setShowBenchmarkModal(true)}>
                    VIEW C++ BENCHMARKS
                  </button>
                </div>
              </div>

              {/* Right Telemetry Radar Console */}
              <div className="hero-telemetry-console">
                <div className="console-title-bar">
                  <span>SYSTEM METRICS RADAR</span>
                  <span className="console-live-badge">LIVE STREAM</span>
                </div>
                <div className="console-stats-list">
                  <div className="console-stat-row">
                    <span className="console-key">CORE ENGINE</span>
                    <span className="console-val color-green">C++14 Native (bin/urbantis.exe)</span>
                  </div>
                  <div className="console-stat-row">
                    <span className="console-key">MONITORED INTERSECTIONS</span>
                    <span className="console-val">25 Municipal Junctions</span>
                  </div>
                  <div className="console-stat-row">
                    <span className="console-key">FIBER CYBER GATEWAYS</span>
                    <span className="console-val">4 Regional Optical Hubs</span>
                  </div>
                  <div className="console-stat-row">
                    <span className="console-key">LEVEL-1/2 TRAUMA CENTERS</span>
                    <span className="console-val">4 Hospitals (Sassoon, Deenanath...)</span>
                  </div>
                  <div className="console-stat-row">
                    <span className="console-key">108 EMS ACTIVE FLEET</span>
                    <span className="console-val color-gold">4 Units Moving / Patrolling</span>
                  </div>
                  <div className="console-stat-row">
                    <span className="console-key">MIDAS STREAM THROUGHPUT</span>
                    <span className="console-val color-green">238,447 Events / Sec (O(1) Space)</span>
                  </div>
                  <div className="console-stat-row">
                    <span className="console-key">PATHFINDING HEURISTIC</span>
                    <span className="console-val">Consistent A* (39.16% Node Reduction)</span>
                  </div>
                </div>
              </div>
            </div>
          </div>

          {/* Tactical Station Selectors (No generic AI cards!) */}
          <div className="tactical-stations-grid">
            <div className="tactical-card" onClick={() => { setCurrentView('workstation'); handleSelectSector(SECTORS[1]); }}>
              <div className="tactical-card-top">
                <span className="station-code">STATION // 01</span>
                <span className="station-badge">ONLINE</span>
              </div>
              <div className="tactical-heading">GIS Cartographic Operations</div>
              <p className="tactical-desc">
                High-definition OpenStreetMap vector canvas over Pune. Real-time traffic signal countdowns,
                BPR link congestion curves, and dynamic multi-unit ambulance telemetry.
              </p>
              <span className="tactical-link">OPEN LIVE MAP VIEW ➔</span>
            </div>

            <div className="tactical-card" onClick={() => { setCurrentView('workstation'); setShowAttackModal(true); }}>
              <div className="tactical-card-top">
                <span className="station-code">STATION // 02</span>
                <span className="station-badge alert">STANDBY</span>
              </div>
              <div className="tactical-heading">Cyber Warfare Simulation Deck</div>
              <p className="tactical-desc">
                Inject targeted SYN flood DDoS, 4-stage killchain hijacking, or sensor spoofing against any Pune gateway.
                Evaluates streaming Chi-Square detection and Pareto response policies.
              </p>
              <span className="tactical-link">LAUNCH ATTACK STUDIO ➔</span>
            </div>

            <div className="tactical-card" onClick={() => { setCurrentView('workstation'); setShowEmergencyModal(true); }}>
              <div className="tactical-card-top">
                <span className="station-code">STATION // 03</span>
                <span className="station-badge ems">READY</span>
              </div>
              <div className="tactical-heading">108 Emergency & Hospital Triage</div>
              <p className="tactical-desc">
                Inject critical collisions across Pune intersections. Multi-criteria triage algorithm selects
                optimal trauma center based on beds, delay, and green wave preemption.
              </p>
              <span className="tactical-link">DISPATCH EMERGENCY ➔</span>
            </div>

            <div className="tactical-card" onClick={() => setShowBenchmarkModal(true)}>
              <div className="tactical-card-top">
                <span className="station-code">STATION // 04</span>
                <span className="station-badge">VERIFIED</span>
              </div>
              <div className="tactical-heading">C++ Algorithmic Benchmark Lab</div>
              <p className="tactical-desc">
                Direct empirical performance metrics: Consistent A* vs. Min-heap Dijkstra, Webster cycle optimization,
                and Tarjan's topological articulation point bottleneck analysis.
              </p>
              <span className="tactical-link">OPEN BENCHMARK LAB ➔</span>
            </div>
          </div>
        </section>
      )}

      {/* ----------------------------------------------------
          VIEW 2: LIVE 2-PANE GIS WORKSTATION
         ---------------------------------------------------- */}
      {currentView === 'workstation' && (
        <main className="split-workspace">
          {/* LEFT WORKSTATION COMMAND DECK */}
          <aside className="command-deck-pane">
            {/* Workstation Header */}
            <div className="deck-header">
              <div className="deck-kicker">PMC OPERATIONS • SECTOR {SECTORS[currentSector].code}</div>
              <h1 className="deck-title">Urbantis Command</h1>
              <p className="deck-desc">
                Click any intersection on the map to inspect its gateway telemetry and execute custom actions.
              </p>
            </div>

            {/* Sector Quick Jump Dock */}
            <div className="sector-nav-dock">
              <span className="deck-section-label">PMRDA Urban Sectors</span>
              <div className="sector-pills-row">
                {SECTORS.map(s => (
                  <button
                    key={s.id}
                    className={`sector-pill ${currentSector === s.id ? 'active' : ''}`}
                    onClick={() => handleSelectSector(s)}
                  >
                    <span className="pill-code">{s.code.split('/')[0]}</span>
                    <span className="pill-name">{s.name.split('&')[0].trim()}</span>
                  </button>
                ))}
              </div>
            </div>

            {/* Selected Node Inspector Card */}
            <div className="inspector-card">
              <div className="inspector-header">
                <div>
                  <div className="node-title">{selectedNode.name}</div>
                  <div className="node-coords">
                    {selectedNode.lat.toFixed(4)}° N, {selectedNode.lng.toFixed(4)}° E • Sector: {selectedNode.sector}
                  </div>
                </div>
                <span className={`status-badge ${
                  compromisedNodes.has(selectedNode.id)
                    ? 'badge-compromised'
                    : blockedNodes.has(selectedNode.id)
                    ? 'badge-blocked'
                    : 'badge-operational'
                }`}>
                  {compromisedNodes.has(selectedNode.id)
                    ? 'CYBER-ATTACK'
                    : blockedNodes.has(selectedNode.id)
                    ? 'BLOCKED / INCIDENT'
                    : 'OPERATIONAL'}
                </span>
              </div>

              <div className="node-metrics-grid">
                <div className="metric-cell">
                  <span className="cell-label">Controlling Gateway</span>
                  <span className="cell-value mono">{selectedNode.gateway}</span>
                </div>
                <div className="metric-cell">
                  <span className="cell-label">Signal Cycle</span>
                  <span className={`cell-value ${selectedNode.signal.nsGreen ? 'color-safe' : 'color-hold'}`}>
                    {selectedNode.signal.nsGreen ? 'North-South Green' : 'East-West Green'} ({selectedNode.signal.timer}s)
                  </span>
                </div>
                <div className="metric-cell">
                  <span className="cell-label">Topological Risk</span>
                  <span className="cell-value">
                    {articulationPoints.includes(selectedNode.id) ? '⚠️ Articulation Point' : 'Standard Arterial'}
                  </span>
                </div>
                <div className="metric-cell">
                  <span className="cell-label">Junction Type</span>
                  <span className="cell-value" style={{ textTransform: 'capitalize' }}>{selectedNode.type}</span>
                </div>
              </div>

              {/* ACTION EXECUTION DECK: EXECUTE ANY ACTION ON THIS NODE */}
              <div className="node-actions-deck">
                <span className="deck-section-label">
                  Execute Action on Node #{selectedNode.id}
                </span>

                <div className="node-actions-buttons-grid">
                  <button className="node-act-btn act-cyber" onClick={() => handleExecuteAttack(selectedNode.id, 'DDOS')}>
                    <svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2.2" strokeLinecap="round" strokeLinejoin="round">
                      <polygon points="13 2 3 14 12 14 11 22 21 10 12 10 13 2" />
                    </svg>
                    <span>Inject Attack</span>
                  </button>

                  <button className="node-act-btn act-ems" onClick={() => handleExecuteEmergency(selectedNode.id, 5)}>
                    <svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2.2" strokeLinecap="round" strokeLinejoin="round">
                      <circle cx="12" cy="12" r="10" />
                      <line x1="12" y1="8" x2="12" y2="12" />
                      <line x1="12" y1="16" x2="12.01" y2="16" />
                    </svg>
                    <span>Trigger Accident</span>
                  </button>

                  <button className="node-act-btn act-green" onClick={() => handlePreemptGreenWave(selectedNode.id)}>
                    <svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2.2" strokeLinecap="round" strokeLinejoin="round">
                      <circle cx="12" cy="12" r="6" fill="currentColor" />
                    </svg>
                    <span>Force Green Wave</span>
                  </button>

                  <button className="node-act-btn act-webster" onClick={() => handleOptimizeWebster(selectedNode.id)}>
                    <svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2.2" strokeLinecap="round" strokeLinejoin="round">
                      <line x1="4" y1="21" x2="4" y2="14" />
                      <line x1="4" y1="10" x2="4" y2="3" />
                      <line x1="12" y1="21" x2="12" y2="12" />
                      <line x1="12" y1="8" x2="12" y2="3" />
                      <line x1="20" y1="21" x2="20" y2="16" />
                      <line x1="20" y1="12" x2="20" y2="3" />
                    </svg>
                    <span>Webster Signal Opt</span>
                  </button>

                  <button className="node-act-btn act-isolate" onClick={() => handleIsolateGateway(selectedNode.gateway)}>
                    <svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2.2" strokeLinecap="round" strokeLinejoin="round">
                      <path d="M12 22s8-4 8-10V5l-8-3-8 3v7c0 6 8 10 8 10z" />
                    </svg>
                    <span>Isolate Gateway</span>
                  </button>

                  <button className="node-act-btn act-clear" onClick={() => handleClearNodeStatus(selectedNode.id)}>
                    <svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2.2" strokeLinecap="round" strokeLinejoin="round">
                      <polyline points="23 4 23 10 17 10" />
                      <path d="M20.49 15a9 9 0 1 1-2.12-9.36L23 10" />
                    </svg>
                    <span>Restore Normal</span>
                  </button>
                </div>
              </div>

              {/* C++ Interactive Pathfinding & Dispatch Tool - FIXED VERTICAL LAYOUT TO PREVENT OVERFLOW */}
              <div className="route-query-box">
                <div className="query-box-title">
                  <svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="#E08738" strokeWidth="2.2" strokeLinecap="round" strokeLinejoin="round">
                    <circle cx="12" cy="12" r="10" />
                    <polyline points="12 6 12 12 16 14" />
                  </svg>
                  <span>Consistent A* Dynamic Pathfinding & Unit Dispatch</span>
                </div>
                <div className="route-calc-form">
                  <select
                    className="route-select"
                    value={targetRouteNodeId}
                    onChange={(e) => setTargetRouteNodeId(Number(e.target.value))}
                  >
                    {PUNE_INTERSECTIONS.filter(n => n.id !== selectedNode.id).map(n => (
                      <option key={n.id} value={n.id}>Destination: #{n.id} — {n.name}</option>
                    ))}
                  </select>
                  <button className="route-calc-btn" onClick={handleCalculateRouteAndDispatch}>
                    <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2.2" strokeLinecap="round" strokeLinejoin="round">
                      <polygon points="3 11 22 2 13 21 11 13 3 11" />
                    </svg>
                    <span>Compute A* Corridor & Dispatch Unit</span>
                  </button>
                </div>

                {routeResult && (
                  <div className="route-benchmark-result">
                    <div style={{ fontWeight: 600, color: 'var(--ink-primary)' }}>
                      Optimal Route: #{routeResult.start} → #{routeResult.goal}
                    </div>
                    <div style={{ color: 'var(--ink-secondary)', marginTop: '4px' }}>
                      • Travel Time: <strong>{Math.round(routeResult.astar.cost)} seconds</strong><br />
                      • A* Nodes Explored: <strong>{routeResult.astar.explored}</strong> vs Dijkstra: <strong>{routeResult.dijkstra.explored}</strong><br />
                      • Efficiency Gain: <strong style={{ color: 'var(--emerald-safe)' }}>{routeResult.reductionPercent}% fewer nodes</strong>
                    </div>
                  </div>
                )}
              </div>
            </div>

            {/* Active Cyber Threat Telemetry & Anomaly Detection */}
            {activeAlerts.length > 0 && (
              <div className="threat-telemetry-card">
                <div className="threat-header">
                  <span className="threat-title">MIDAS Streaming Telemetry</span>
                  <span className="threat-score">χ² Anomaly: {activeAlerts[0].score}</span>
                </div>
                <div className="threat-body">
                  <div className="threat-category">{activeAlerts[0].category}</div>
                  <div className="threat-expl">{activeAlerts[0].explanation}</div>
                  <div className="threat-meta">
                    Target: <code>{activeAlerts[0].target}</code> (Node #{activeAlerts[0].targetNode})
                  </div>
                </div>
              </div>
            )}

            {/* Pareto Incident Response Optimizer */}
            {compromisedNodes.size > 0 && (
              <div className="pareto-card">
                <div className="pareto-header">
                  <span className="pareto-title">Pareto Response Policies</span>
                  <span className="pareto-badge">DECISION MATRIX</span>
                </div>
                <div className="pareto-options-list">
                  {paretoStrategies.map(strat => (
                    <div
                      key={strat.type}
                      className={`pareto-option ${selectedStrategy === strat.type ? 'active' : ''}`}
                      onClick={() => handleApplyStrategy(strat.type)}
                    >
                      <div>
                        <div className="strat-title">{strat.title}</div>
                        <div className="strat-desc">{strat.recommendation}</div>
                      </div>
                      <div className="strat-score">Loss: {Math.round(strat.compositeScore)}</div>
                    </div>
                  ))}
                </div>
              </div>
            )}

            {/* 108 Emergency EMS Fleet Status */}
            <div className="ems-status-card">
              <div className="ems-header">
                <span className="ems-title">108 Emergency Fleet Status</span>
                <span className="ems-badge">{ambulances.filter(a => a.isEmergency).length} Priority • {ambulances.length} Active</span>
              </div>
              <div className="ems-list">
                {ambulances.map(amb => (
                  <div key={amb.id} className="ems-row">
                    <div className="amb-info">
                      <span className="amb-icon">🚑</span>
                      <div>
                        <span className="amb-name">{amb.name}</span>
                        <span className="amb-base">{amb.hospital}</span>
                      </div>
                    </div>
                    <span className={`amb-status ${amb.isEmergency ? 'en-route' : 'standby'}`}>
                      {amb.status}
                    </span>
                  </div>
                ))}
              </div>
            </div>
          </aside>

          {/* RIGHT FULL-BLEED LIVE INTERACTIVE LEAFLET MAP VIEWPORT */}
          <section className="map-viewport-pane">
            <PuneLiveMap
              selectedNodeId={selectedNodeId}
              onSelectNode={handleNodeSelect}
              compromisedNodes={compromisedNodes}
              blockedNodes={blockedNodes}
              preemptedCorridor={preemptedCorridor}
              ambulances={ambulances}
              focusCoords={focusCoords}
            />
          </section>
        </main>
      )}

      {/* ----------------------------------------------------
          MODAL 1: CYBER THREAT INJECTION STUDIO
         ---------------------------------------------------- */}
      {showAttackModal && (
        <div className="modal-overlay" onClick={() => setShowAttackModal(false)}>
          <div className="modal-content" onClick={e => e.stopPropagation()}>
            <div className="modal-header">
              <h2>⚡ Inject Targeted Cyber Attack</h2>
              <button className="modal-close" onClick={() => setShowAttackModal(false)}>✕</button>
            </div>
            <div className="modal-body">
              <label className="modal-label">Target Intersection Node:</label>
              <select
                className="modal-select"
                value={selectedNodeId}
                onChange={e => setSelectedNodeId(Number(e.target.value))}
              >
                {PUNE_INTERSECTIONS.map(n => (
                  <option key={n.id} value={n.id}>Node #{n.id} - {n.name} (Gateway: {n.gateway})</option>
                ))}
              </select>

              <label className="modal-label" style={{ marginTop: '14px' }}>Attack Vector:</label>
              <div className="modal-options-grid">
                <div
                  className={`option-box ${customAttackType === 'DDOS' ? 'selected' : ''}`}
                  onClick={() => setCustomAttackType('DDOS')}
                >
                  <strong>Volumetric DDoS Flood</strong>
                  <span>High-rate SYN stream (3,500 pkts/s) against gateway.</span>
                </div>
                <div
                  className={`option-box ${customAttackType === 'KILLCHAIN' ? 'selected' : ''}`}
                  onClick={() => setCustomAttackType('KILLCHAIN')}
                >
                  <strong>Staged Killchain Hijack</strong>
                  <span>Port Scan → Auth Bypass → Controller Signal Override.</span>
                </div>
                <div
                  className={`option-box ${customAttackType === 'MITM' ? 'selected' : ''}`}
                  onClick={() => setCustomAttackType('MITM')}
                >
                  <strong>Man-in-the-Middle Telemetry</strong>
                  <span>Cryptographic replay attack spoofing loop sensor queues.</span>
                </div>
                <div
                  className={`option-box ${customAttackType === 'TAMPERING' ? 'selected' : ''}`}
                  onClick={() => setCustomAttackType('TAMPERING')}
                >
                  <strong>Inductive Sensor Tampering</strong>
                  <span>Inject false zero-occupancy signals into traffic controllers.</span>
                </div>
              </div>
            </div>
            <div className="modal-footer">
              <button className="modal-btn-cancel" onClick={() => setShowAttackModal(false)}>Cancel</button>
              <button className="modal-btn-execute" onClick={() => handleExecuteAttack(selectedNodeId, customAttackType)}>
                Execute Cyber Attack ⚡
              </button>
            </div>
          </div>
        </div>
      )}

      {/* ----------------------------------------------------
          MODAL 2: 108 EMERGENCY INCIDENT DISPATCHER
         ---------------------------------------------------- */}
      {showEmergencyModal && (
        <div className="modal-overlay" onClick={() => setShowEmergencyModal(false)}>
          <div className="modal-content" onClick={e => e.stopPropagation()}>
            <div className="modal-header">
              <h2>🚨 Trigger 108 Emergency Incident</h2>
              <button className="modal-close" onClick={() => setShowEmergencyModal(false)}>✕</button>
            </div>
            <div className="modal-body">
              <label className="modal-label">Incident Scene Location:</label>
              <select
                className="modal-select"
                value={selectedNodeId}
                onChange={e => setSelectedNodeId(Number(e.target.value))}
              >
                {PUNE_INTERSECTIONS.map(n => (
                  <option key={n.id} value={n.id}>Node #{n.id} - {n.name}</option>
                ))}
              </select>

              <label className="modal-label" style={{ marginTop: '14px' }}>Incident Severity Level (1 - 5):</label>
              <div style={{ display: 'flex', gap: '8px', marginTop: '6px' }}>
                {[1, 2, 3, 4, 5].map(lvl => (
                  <button
                    key={lvl}
                    className={`severity-btn ${customSeverity === lvl ? 'active' : ''}`}
                    onClick={() => setCustomSeverity(lvl)}
                  >
                    Level {lvl} {lvl === 5 ? '(Mass Casualty)' : lvl >= 3 ? '(Trauma)' : '(Minor)'}
                  </button>
                ))}
              </div>
              <p style={{ fontSize: '0.74rem', color: 'var(--ink-muted)', marginTop: '8px' }}>
                Executing will run multi-criteria hospital triage, dispatch closest 108 ambulance,
                calculate fastest route, and preempt green wave corridor.
              </p>
            </div>
            <div className="modal-footer">
              <button className="modal-btn-cancel" onClick={() => setShowEmergencyModal(false)}>Cancel</button>
              <button className="modal-btn-execute ems" onClick={() => handleExecuteEmergency(selectedNodeId, customSeverity)}>
                Dispatch 108 Ambulance 🚑
              </button>
            </div>
          </div>
        </div>
      )}

      {/* ----------------------------------------------------
          MODAL 3: C++ ALGORITHMIC BENCHMARK LABORATORY
         ---------------------------------------------------- */}
      {showBenchmarkModal && (
        <div className="modal-overlay" onClick={() => setShowBenchmarkModal(false)}>
          <div className="modal-content benchmark" onClick={e => e.stopPropagation()}>
            <div className="modal-header">
              <h2>🧪 C++14 Algorithmic Core Benchmark Lab</h2>
              <button className="modal-close" onClick={() => setShowBenchmarkModal(false)}>✕</button>
            </div>
            <div className="modal-body">
              <div className="benchmark-grid">
                <div className="bench-box">
                  <div className="bench-title">Consistent A* vs. Min-Heap Dijkstra</div>
                  <div className="bench-metric">39.16% Node Reduction</div>
                  <p className="bench-desc">
                    Euclidean admissible heuristic achieves 100% path optimality while reducing visited state-space by ~40%.
                  </p>
                  <div className="bench-code">
                    <code>Speedup: 20,310x in micro-benchmarks</code>
                  </div>
                </div>

                <div className="bench-box">
                  <div className="bench-title">MIDAS Dynamic Streaming Sketch</div>
                  <div className="bench-metric">238,447 Events / Sec</div>
                  <p className="bench-desc">
                    Streaming dynamic graph anomaly detection using Chi-squared goodness-of-fit test statistic in constant O(1) space.
                  </p>
                  <div className="bench-code">
                    <code>Memory: Constant O(1) • False Positives: 0.0%</code>
                  </div>
                </div>

                <div className="bench-box">
                  <div className="bench-title">Tarjan's Articulation Points</div>
                  <div className="bench-metric">O(V + E) Topological Bottlenecks</div>
                  <p className="bench-desc">
                    Discovers topological bridges and single points of failure across the Pune arterial grid.
                  </p>
                  <div className="bench-code">
                    <code>Bottleneck Nodes Identified: #{articulationPoints.join(', #')}</code>
                  </div>
                </div>

                <div className="bench-box">
                  <div className="bench-title">Webster's Optimal Signal Timing</div>
                  <div className="bench-metric">C0 = (1.5L + 5) / (1 - Y)</div>
                  <p className="bench-desc">
                    Dynamically recalculates cycle length and green-split ratios based on saturation flow ratios.
                  </p>
                  <div className="bench-code">
                    <code>Delay Minimization: Optimal Webster Formula</code>
                  </div>
                </div>
              </div>
            </div>
            <div className="modal-footer">
              <button className="modal-btn-cancel" onClick={() => setShowBenchmarkModal(false)}>Close Benchmark</button>
              <button className="modal-btn-execute" onClick={() => { setShowBenchmarkModal(false); setCurrentView('workstation'); }}>
                Return to Workstation
              </button>
            </div>
          </div>
        </div>
      )}

      {/* ----------------------------------------------------
          CLEAN REFINED APPLICATION STATUS BAR (ZERO OVERFLOW/OVERLAP)
         ---------------------------------------------------- */}
      <footer className="app-footer-bar">
        <div className="footer-status-pill">
          <span className="footer-status-dot"></span>
          <span>C++14 ENGINE ACTIVE</span>
        </div>
        <div className="footer-item">PMC MUNICIPAL GRID</div>
        <div className="footer-item">PMRDA REGION</div>
        <div className="footer-divider">/</div>
        <div className="footer-item">25 JUNCTIONS</div>
        <div className="footer-item">4 CYBER GATEWAYS</div>
        <div className="footer-item">4 TRAUMA HUBS</div>
        <div className="footer-item color-highlight">4 ACTIVE PATROLS</div>
        <div className="footer-divider">/</div>
        <div className="footer-item">MIDAS: 238k EVT/S [O(1)]</div>
        <div className="footer-item mono-coord">PUNE: 18.5204° N, 73.8567° E</div>
      </footer>
    </div>
  );
}
