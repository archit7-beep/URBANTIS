// Pune Smart City Network, Geography, Cyber Topology, and Simulation Primitives (English Only)

export const PUNE_INTERSECTIONS = [
  { id: 1, name: "Pune University Circle", lat: 18.5529, lng: 73.8260, gateway: "SRV-PMC-CORE", sector: "North-West", type: "hub", signal: { nsGreen: true, timer: 24, max: 45 } },
  { id: 2, name: "Shivajinagar / Shimla Office", lat: 18.5314, lng: 73.8446, gateway: "SRV-PMC-CORE", sector: "Central", type: "metro", signal: { nsGreen: false, timer: 18, max: 40 } },
  { id: 3, name: "Pune Station / Sassoon Chowk", lat: 18.5284, lng: 73.8742, gateway: "SRV-PMC-CORE", sector: "Central-East", type: "hospital", hospitalId: 1, signal: { nsGreen: true, timer: 32, max: 50 } },
  { id: 4, name: "Yerawada / Bund Garden", lat: 18.5526, lng: 73.8825, gateway: "GW-MAGARPATTA-02", sector: "North-East", type: "bridge", hospitalId: 3, signal: { nsGreen: true, timer: 20, max: 40 } },
  { id: 5, name: "Viman Nagar / Phoenix Chowk", lat: 18.5679, lng: 73.9143, gateway: "GW-MAGARPATTA-02", sector: "Airport-Corridor", type: "arterial", signal: { nsGreen: false, timer: 15, max: 35 } },
  { id: 6, name: "Senapati Bapat Road (ICC)", lat: 18.5362, lng: 73.8300, gateway: "SRV-PMC-CORE", sector: "West-Commercial", type: "arterial", signal: { nsGreen: true, timer: 28, max: 45 } },
  { id: 7, name: "FC Road / Deccan Gymkhana", lat: 18.5186, lng: 73.8415, gateway: "SRV-PMC-CORE", sector: "Cultural-Core", type: "hospital", hospitalId: 4, signal: { nsGreen: true, timer: 22, max: 40 } },
  { id: 8, name: "JM Road / Sambhaji Bridge", lat: 18.5204, lng: 73.8496, gateway: "SRV-PMC-CORE", sector: "Central", type: "arterial", signal: { nsGreen: false, timer: 19, max: 35 } },
  { id: 9, name: "Camp / MG Road Chowk", lat: 18.5158, lng: 73.8797, gateway: "GW-MAGARPATTA-02", sector: "Cantonment", type: "arterial", signal: { nsGreen: true, timer: 26, max: 40 } },
  { id: 10, name: "Kalyani Nagar Chowk", lat: 18.5463, lng: 73.9034, gateway: "GW-MAGARPATTA-02", sector: "East-Residential", type: "bridge", signal: { nsGreen: false, timer: 21, max: 40 } },
  { id: 11, name: "Kothrud / Paud Phata", lat: 18.5074, lng: 73.8077, gateway: "GW-HINJEWADI-01", sector: "West-Suburbs", type: "residential", signal: { nsGreen: true, timer: 25, max: 45 } },
  { id: 12, name: "Deenanath Mangeshkar Chowk", lat: 18.5018, lng: 73.8278, gateway: "GW-HINJEWADI-01", sector: "South-West", type: "hospital", hospitalId: 2, signal: { nsGreen: true, timer: 30, max: 50 } },
  { id: 13, name: "Swargate Junction", lat: 18.5018, lng: 73.8585, gateway: "GW-SWARGATE-03", sector: "Transit-Hub", type: "terminal", signal: { nsGreen: false, timer: 14, max: 50 } },
  { id: 14, name: "Fatima Nagar / Wanowrie", lat: 18.5033, lng: 73.9011, gateway: "GW-MAGARPATTA-02", sector: "South-East", type: "arterial", signal: { nsGreen: true, timer: 27, max: 40 } },
  { id: 15, name: "Hadapsar / Magarpatta City", lat: 18.5089, lng: 73.9259, gateway: "GW-MAGARPATTA-02", sector: "East-IT-Hub", type: "cyber-hub", signal: { nsGreen: false, timer: 22, max: 45 } },
  { id: 16, name: "Chandani Chowk (NH-48)", lat: 18.5074, lng: 73.7828, gateway: "GW-HINJEWADI-01", sector: "Highway-Bypass", type: "flyover", signal: { nsGreen: true, timer: 35, max: 60 } },
  { id: 17, name: "Warje Flyover Chowk", lat: 18.4797, lng: 73.8016, gateway: "GW-HINJEWADI-01", sector: "South-Highway", type: "flyover", signal: { nsGreen: true, timer: 24, max: 45 } },
  { id: 18, name: "Sinhagad Road / Sarasbaug", lat: 18.4988, lng: 73.8519, gateway: "GW-SWARGATE-03", sector: "South", type: "arterial", signal: { nsGreen: false, timer: 18, max: 40 } },
  { id: 19, name: "Market Yard / Gultekdi", lat: 18.4891, lng: 73.8682, gateway: "GW-SWARGATE-03", sector: "Logistics", type: "arterial", signal: { nsGreen: true, timer: 25, max: 40 } },
  { id: 20, name: "Kondhwa / Lullanagar", lat: 18.4839, lng: 73.8906, gateway: "GW-SWARGATE-03", sector: "South-East", type: "residential", signal: { nsGreen: false, timer: 20, max: 40 } },
  { id: 21, name: "Hinjewadi Phase 1 (Infotech)", lat: 18.5913, lng: 73.7389, gateway: "GW-HINJEWADI-01", sector: "Mega-Cyber-Hub", type: "cyber-hub", signal: { nsGreen: true, timer: 30, max: 55 } },
  { id: 22, name: "Wakad / Dange Chowk", lat: 18.5987, lng: 73.7667, gateway: "GW-HINJEWADI-01", sector: "Pimpri-Pune-Link", type: "arterial", signal: { nsGreen: false, timer: 22, max: 45 } },
  { id: 23, name: "Baner / Balewadi High Street", lat: 18.5590, lng: 73.7868, gateway: "GW-HINJEWADI-01", sector: "North-West", type: "arterial", signal: { nsGreen: true, timer: 26, max: 45 } },
  { id: 24, name: "Katraj Chowk (Wonder City)", lat: 18.4575, lng: 73.8588, gateway: "GW-SWARGATE-03", sector: "South-Gateway", type: "highway", signal: { nsGreen: true, timer: 35, max: 60 } },
  { id: 25, name: "Bibwewadi Chowk", lat: 18.4697, lng: 73.8654, gateway: "GW-SWARGATE-03", sector: "South-Suburbs", type: "arterial", signal: { nsGreen: false, timer: 19, max: 40 } },
];

export const PUNE_ROADS = [
  // IT Corridor (Hinjewadi to University)
  { id: 101, u: 21, v: 22, name: "Hinjewadi-Wakad Expressway", type: "HIGHWAY", speedKmH: 70, cap: 3200, congestion: 0.35 },
  { id: 102, u: 22, v: 23, name: "Wakad-Baner Link Road", type: "ARTERIAL", speedKmH: 55, cap: 2200, congestion: 0.42 },
  { id: 103, u: 23, v: 1, name: "Baner-Pashan University Road", type: "ARTERIAL", speedKmH: 50, cap: 2000, congestion: 0.48 },

  // University to Shivajinagar & Station
  { id: 104, u: 1, v: 2, name: "Ganeshkhind Road", type: "ARTERIAL", speedKmH: 50, cap: 2400, congestion: 0.55 },
  { id: 105, u: 2, v: 3, name: "Sangamvadi / Pune Station Corridor", type: "ARTERIAL", speedKmH: 45, cap: 2200, congestion: 0.62 },

  // Central Hub (S.B. Road, Deccan, FC Road, JM Road)
  { id: 106, u: 1, v: 6, name: "Senapati Bapat Road", type: "ARTERIAL", speedKmH: 45, cap: 1600, congestion: 0.40 },
  { id: 107, u: 6, v: 7, name: "Prabhat / FC Road Link", type: "LOCAL", speedKmH: 35, cap: 1200, congestion: 0.38 },
  { id: 108, u: 2, v: 8, name: "JM Road (One-Way Spine)", type: "ARTERIAL", speedKmH: 50, cap: 2000, congestion: 0.45 },
  { id: 109, u: 7, v: 8, name: "Sambhaji Bridge / Garware Cross", type: "CONNECTOR", speedKmH: 35, cap: 1200, congestion: 0.50 },

  // Eastern River & IT Corridor (Bund Garden, Kalyani Nagar, Viman Nagar)
  { id: 110, u: 3, v: 4, name: "Bund Garden Bridge Road", type: "ARTERIAL", speedKmH: 50, cap: 2200, congestion: 0.46 },
  { id: 111, u: 4, v: 10, name: "Koregaon Park / Kalyani Nagar Link", type: "ARTERIAL", speedKmH: 45, cap: 1800, congestion: 0.40 },
  { id: 112, u: 10, v: 5, name: "Ahmednagar Highway / Viman Nagar", type: "ARTERIAL", speedKmH: 55, cap: 2400, congestion: 0.52 },

  // Magarpatta & Camp Corridors
  { id: 113, u: 3, v: 9, name: "Station-Camp MG Road", type: "ARTERIAL", speedKmH: 40, cap: 1600, congestion: 0.48 },
  { id: 114, u: 9, v: 14, name: "Solapur Road / Fatima Nagar", type: "ARTERIAL", speedKmH: 45, cap: 1800, congestion: 0.50 },
  { id: 115, u: 14, v: 15, name: "Magarpatta Cyber City Spine", type: "ARTERIAL", speedKmH: 50, cap: 2200, congestion: 0.58 },
  { id: 116, u: 10, v: 15, name: "Mundhwa-Hadapsar Connector", type: "CONNECTOR", speedKmH: 40, cap: 1400, congestion: 0.44 },

  // Kothrud & Karve Road to Deccan
  { id: 117, u: 11, v: 12, name: "Paud Phata - Erandwane Link", type: "ARTERIAL", speedKmH: 45, cap: 1600, congestion: 0.42 },
  { id: 118, u: 12, v: 7, name: "Karve Road to Deccan Gymkhana", type: "ARTERIAL", speedKmH: 40, cap: 1800, congestion: 0.65 },

  // Central to Swargate Hub
  { id: 119, u: 8, v: 13, name: "Bajirao Road to Swargate", type: "ARTERIAL", speedKmH: 45, cap: 2200, congestion: 0.68 },
  { id: 120, u: 7, v: 13, name: "Tilak Road to Swargate", type: "ARTERIAL", speedKmH: 40, cap: 1800, congestion: 0.60 },

  // Swargate to South Suburbs
  { id: 121, u: 13, v: 18, name: "Sarasbaug / Mitra Mandal Road", type: "LOCAL", speedKmH: 35, cap: 1400, congestion: 0.40 },
  { id: 122, u: 13, v: 19, name: "Market Yard Commercial Road", type: "ARTERIAL", speedKmH: 40, cap: 1800, congestion: 0.52 },
  { id: 123, u: 19, v: 20, name: "Bibwewadi-Kondhwa Link", type: "LOCAL", speedKmH: 35, cap: 1400, congestion: 0.38 },
  { id: 124, u: 14, v: 20, name: "Salunke Vihar / Kondhwa Cross", type: "CONNECTOR", speedKmH: 35, cap: 1200, congestion: 0.42 },

  // South Perimeter
  { id: 125, u: 13, v: 25, name: "Satara Road Spine to Bibwewadi", type: "ARTERIAL", speedKmH: 50, cap: 2400, congestion: 0.58 },
  { id: 126, u: 25, v: 24, name: "Satara Road to Katraj Chowk", type: "ARTERIAL", speedKmH: 50, cap: 2200, congestion: 0.64 },
  { id: 127, u: 18, v: 24, name: "Sinhagad Road Peripheral Link", type: "ARTERIAL", speedKmH: 45, cap: 1800, congestion: 0.46 },

  // Western Express Ring Road Bypass (NH-48 Mumbai-Bangalore Highway)
  { id: 128, u: 21, v: 16, name: "NH-48 Hinjewadi-Chandani Bypass", type: "HIGHWAY", speedKmH: 80, cap: 3600, congestion: 0.40 },
  { id: 129, u: 11, v: 16, name: "Kothrud-Chandani Chowk Connector", type: "ARTERIAL", speedKmH: 50, cap: 1800, congestion: 0.45 },
  { id: 130, u: 16, v: 17, name: "NH-48 Chandani Chowk to Warje Flyover", type: "HIGHWAY", speedKmH: 80, cap: 3600, congestion: 0.48 },
  { id: 131, u: 17, v: 24, name: "NH-48 Warje to Katraj Bypass Tunnel", type: "HIGHWAY", speedKmH: 85, cap: 3800, congestion: 0.38 },
];

export const PUNE_HOSPITALS = [
  { id: 1, name: "Sassoon General Hospital", intersectionId: 3, traumaLevel: 1, totalBeds: 250, availableBeds: 54, triageDelaySec: 45 },
  { id: 2, name: "Deenanath Mangeshkar Hospital", intersectionId: 12, traumaLevel: 1, totalBeds: 200, availableBeds: 38, triageDelaySec: 70 },
  { id: 3, name: "Ruby Hall Clinic", intersectionId: 4, traumaLevel: 2, totalBeds: 180, availableBeds: 26, triageDelaySec: 60 },
  { id: 4, name: "Sahyadri Super Speciality Hospital", intersectionId: 7, traumaLevel: 2, totalBeds: 140, availableBeds: 21, triageDelaySec: 50 },
];

export const PUNE_GATEWAYS = [
  { id: "SRV-PMC-CORE", name: "PMC Central Smart City Hub", location: "Shivajinagar", sector: "Central", status: "ONLINE", boundNodes: [1, 2, 3, 6, 7, 8] },
  { id: "GW-HINJEWADI-01", name: "Hinjewadi Infotech Gateway", location: "Phase 1 Infotech", sector: "West", status: "ONLINE", boundNodes: [11, 12, 16, 17, 21, 22, 23] },
  { id: "GW-MAGARPATTA-02", name: "Magarpatta Cyber Gateway", location: "Hadapsar Cyber City", sector: "East", status: "ONLINE", boundNodes: [4, 5, 9, 10, 14, 15] },
  { id: "GW-SWARGATE-03", name: "Swargate Regional Transit Hub", location: "Swargate Junction", sector: "South", status: "ONLINE", boundNodes: [13, 18, 19, 20, 24, 25] },
];

export const INITIAL_AMBULANCES = [
  {
    id: 1,
    name: "Pune-108-Alpha",
    hospital: "Sassoon Hospital",
    currentNode: 3,
    targetNode: 2,
    progress: 0.1,
    lat: 18.5284,
    lng: 73.8742,
    status: "ACTIVE PATROL // CENTRAL CORE",
    isEmergency: false,
    speedKmH: 60,
    patrolCircuit: [3, 2, 8, 7, 6, 1, 2, 3],
    patrolIndex: 0,
    path: []
  },
  {
    id: 2,
    name: "Pune-108-Bravo",
    hospital: "Deenanath Mangeshkar",
    currentNode: 12,
    targetNode: 11,
    progress: 0.25,
    lat: 18.5018,
    lng: 73.8278,
    status: "ACTIVE PATROL // KOTHRUD ARTERIAL",
    isEmergency: false,
    speedKmH: 60,
    patrolCircuit: [12, 11, 16, 17, 24, 25, 13, 7, 12],
    patrolIndex: 0,
    path: []
  },
  {
    id: 3,
    name: "Pune-108-Charlie",
    hospital: "Swargate Transit Hub",
    currentNode: 13,
    targetNode: 18,
    progress: 0.4,
    lat: 18.5018,
    lng: 73.8585,
    status: "ACTIVE PATROL // SWARGATE-KATRAJ",
    isEmergency: false,
    speedKmH: 60,
    patrolCircuit: [13, 18, 24, 25, 19, 13],
    patrolIndex: 0,
    path: []
  },
  {
    id: 4,
    name: "Pune-108-Delta",
    hospital: "Hinjewadi Infotech",
    currentNode: 21,
    targetNode: 22,
    progress: 0.15,
    lat: 18.5913,
    lng: 73.7389,
    status: "ACTIVE PATROL // HINJEWADI EXPRESSWAY",
    isEmergency: false,
    speedKmH: 65,
    patrolCircuit: [21, 22, 23, 1, 23, 22, 21],
    patrolIndex: 0,
    path: []
  },
];
