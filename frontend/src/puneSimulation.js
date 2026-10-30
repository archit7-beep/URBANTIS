// Pure JS Algorithmic Engine matching C++ URBANTIS DSA implementations

import { PUNE_INTERSECTIONS, PUNE_ROADS, PUNE_HOSPITALS } from './puneData';

// ------------------------------------------------------------------
// 1. Graph Algorithms: Dijkstra & Consistent A*
// ------------------------------------------------------------------
export function buildAdjacencyList() {
  const adj = {};
  PUNE_INTERSECTIONS.forEach(n => { adj[n.id] = []; });

  PUNE_ROADS.forEach(road => {
    // Dynamic travel time in seconds: BPR formula
    const freeFlowSec = (road.speedKmH > 0) ? (1200 / (road.speedKmH / 3.6)) : 60;
    const vcr = road.congestion;
    const dynamicTime = freeFlowSec * (1 + 0.15 * Math.pow(vcr, 4));

    adj[road.u].push({ to: road.v, roadId: road.id, cost: dynamicTime, length: 1200, road });
    adj[road.v].push({ to: road.u, roadId: road.id, cost: dynamicTime, length: 1200, road });
  });

  return adj;
}

export function computeAStar(startId, goalId, blockedNodes = new Set()) {
  const adj = buildAdjacencyList();
  const startNode = PUNE_INTERSECTIONS.find(n => n.id === startId);
  const goalNode = PUNE_INTERSECTIONS.find(n => n.id === goalId);
  if (!startNode || !goalNode) return { path: [], cost: 0, explored: 0 };

  const heuristic = (currId) => {
    const curr = PUNE_INTERSECTIONS.find(n => n.id === currId);
    if (!curr) return 0;
    const dLat = (curr.lat - goalNode.lat) * 111000;
    const dLng = (curr.lng - goalNode.lng) * 105000;
    const dist = Math.sqrt(dLat * dLat + dLng * dLng);
    return dist / 25.0; // Max speed lower bound (admissible, m/s)
  };

  const gScore = {};
  const parent = {};
  const pq = [{ id: startId, f: heuristic(startId), g: 0 }];
  gScore[startId] = 0;
  let explored = 0;

  while (pq.length > 0) {
    pq.sort((a, b) => a.f - b.f);
    const { id: u, g: currentG } = pq.shift();
    explored++;

    if (u === goalId) {
      const path = [];
      let curr = goalId;
      while (curr !== undefined) {
        path.unshift(curr);
        curr = parent[curr];
      }
      return { path, cost: gScore[goalId], explored };
    }

    const neighbors = adj[u] || [];
    for (const edge of neighbors) {
      const v = edge.to;
      if (blockedNodes.has(v)) continue;

      const tentativeG = currentG + edge.cost;
      if (gScore[v] === undefined || tentativeG < gScore[v]) {
        gScore[v] = tentativeG;
        parent[v] = u;
        const f = tentativeG + heuristic(v);
        pq.push({ id: v, f, g: tentativeG });
      }
    }
  }

  return { path: [], cost: Infinity, explored };
}

export function computeDijkstra(startId, goalId, blockedNodes = new Set()) {
  const adj = buildAdjacencyList();
  const dist = {};
  const parent = {};
  const pq = [{ id: startId, cost: 0 }];
  dist[startId] = 0;
  let explored = 0;

  while (pq.length > 0) {
    pq.sort((a, b) => a.cost - b.cost);
    const { id: u, cost: currentCost } = pq.shift();
    explored++;

    if (u === goalId) {
      const path = [];
      let curr = goalId;
      while (curr !== undefined) {
        path.unshift(curr);
        curr = parent[curr];
      }
      return { path, cost: dist[goalId], explored };
    }

    const neighbors = adj[u] || [];
    for (const edge of neighbors) {
      const v = edge.to;
      if (blockedNodes.has(v)) continue;

      const alt = currentCost + edge.cost;
      if (dist[v] === undefined || alt < dist[v]) {
        dist[v] = alt;
        parent[v] = u;
        pq.push({ id: v, cost: alt });
      }
    }
  }

  return { path: [], cost: Infinity, explored };
}

// ------------------------------------------------------------------
// 2. Tarjan's Articulation Points Algorithm (Topological Bottlenecks)
// ------------------------------------------------------------------
export function findArticulationPoints() {
  const adj = buildAdjacencyList();
  const visited = {};
  const disc = {};
  const low = {};
  const parent = {};
  const isArt = new Set();
  let timer = 0;

  function dfs(u) {
    visited[u] = true;
    disc[u] = low[u] = ++timer;
    let children = 0;

    for (const edge of adj[u] || []) {
      const v = edge.to;
      if (!visited[v]) {
        children++;
        parent[v] = u;
        dfs(v);
        low[u] = Math.min(low[u], low[v]);

        if (parent[u] === undefined && children > 1) {
          isArt.add(u);
        }
        if (parent[u] !== undefined && low[v] >= disc[u]) {
          isArt.add(u);
        }
      } else if (v !== parent[u]) {
        low[u] = Math.min(low[u], disc[v]);
      }
    }
  }

  PUNE_INTERSECTIONS.forEach(n => {
    if (!visited[n.id]) dfs(n.id);
  });

  return Array.from(isArt);
}

// ------------------------------------------------------------------
// 3. Webster's Method for Signal Timing Optimization
// ------------------------------------------------------------------
export function optimizeSignalWebster(intersectionId) {
  const node = PUNE_INTERSECTIONS.find(n => n.id === intersectionId);
  if (!node) return { optimalCycle: 60, nsGreen: 32, ewGreen: 28 };

  const lostTimePerPhase = 4.0;
  const numPhases = 2;
  const totalLostTime = lostTimePerPhase * numPhases; // L = 8.0s

  const yNS = 0.38;
  const yEW = 0.32;
  const Y = Math.min(0.85, yNS + yEW);

  const rawCycle = (1.5 * totalLostTime + 5.0) / (1.0 - Y);
  const optimalCycle = Math.round(Math.min(120, Math.max(45, rawCycle)));
  const effectiveGreen = optimalCycle - totalLostTime;
  const nsGreen = Math.round(effectiveGreen * (yNS / Y));
  const ewGreen = effectiveGreen - nsGreen;

  return { optimalCycle, nsGreen, ewGreen };
}

// ------------------------------------------------------------------
// 4. Multi-Criteria Hospital Triage Selector
// ------------------------------------------------------------------
export function selectOptimalHospital(sceneNodeId, severity = 4, blockedNodes = new Set()) {
  let bestHospital = null;
  let minLoss = Infinity;

  PUNE_HOSPITALS.forEach(hosp => {
    const route = computeAStar(sceneNodeId, hosp.intersectionId, blockedNodes);
    if (route.path.length === 0) return;

    const travelTime = route.cost;
    const traumaMismatch = (severity >= 4 && hosp.traumaLevel > 1) ? 240 : 0;
    const triageDelay = hosp.triageDelaySec;
    const bedBonus = hosp.availableBeds * 1.5;

    const loss = travelTime + triageDelay + traumaMismatch - bedBonus;
    if (loss < minLoss) {
      minLoss = loss;
      bestHospital = { ...hosp, route, loss };
    }
  });

  return bestHospital;
}

// ------------------------------------------------------------------
// 5. Incident Response Pareto Optimizer
// ------------------------------------------------------------------
export function evaluateParetoStrategies(incidentTarget, anomalyScore, severity) {
  const isHospitalAdjacent = [3, 4, 7, 12].includes(incidentTarget);
  const baseDelay = severity * 18.0;

  const strategies = [
    {
      type: "PASSIVE_MONITOR",
      title: "1. Passive Observation & Monitoring",
      description: "Allow network stream to flow without physical actuation. Zero service disturbance.",
      trafficDelay: baseDelay * 1.8,
      emergencyRisk: isHospitalAdjacent ? 520 : 180,
      cyberExposure: anomalyScore * 24.0 * severity,
      isolationCost: 0,
      recommendation: "Not Recommended. High ongoing risk to Pune traffic controllers."
    },
    {
      type: "AGGRESSIVE_ISOLATION",
      title: "2. Total Sector Hardware Isolation",
      description: "Hard shutdown of affected gateway and revert all bound intersections to flashing 4-way stop.",
      trafficDelay: 320,
      emergencyRisk: isHospitalAdjacent ? 280 : 60,
      cyberExposure: 0,
      isolationCost: 260,
      recommendation: "Eliminates cyber risk but inflicts severe gridlock across arterial corridors."
    },
    {
      type: "SURGICAL_FAILOVER",
      title: "3. Surgical Backup Gateway Failover",
      description: "Hot-migrate signal controller telemetry to secondary gateway with Webster hardcoded fallback.",
      trafficDelay: 75,
      emergencyRisk: 85,
      cyberExposure: severity * 15,
      isolationCost: 40,
      recommendation: "Viable fallback. Fast recovery with low transit disruption."
    },
    {
      type: "EMERGENCY_GREEN_CORRIDOR",
      title: "4. Emergency Corridor Preemption (Recommended)",
      description: "Isolate compromised controller, establish pre-cleared dynamic green wave for 108 Ambulances, and reroute traffic upstream.",
      trafficDelay: 85,
      emergencyRisk: 12,
      cyberExposure: 10,
      isolationCost: 45,
      recommendation: "Pareto-Optimal. Minimizes composite loss while safeguarding emergency lives."
    }
  ];

  strategies.forEach(s => {
    s.compositeScore = (s.trafficDelay * 1.0) +
                       (s.emergencyRisk * 2.5) +
                       (s.cyberExposure * 2.0) +
                       (s.isolationCost * 0.8);
  });

  strategies.sort((a, b) => a.compositeScore - b.compositeScore);
  return strategies;
}
