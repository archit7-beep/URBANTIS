import React, { useEffect, useRef } from 'react';
import L from 'leaflet';
import 'leaflet/dist/leaflet.css';
import { PUNE_INTERSECTIONS, PUNE_ROADS } from './puneData';

export default function PuneLiveMap({
  selectedNodeId,
  onSelectNode,
  compromisedNodes,
  blockedNodes,
  preemptedCorridor,
  ambulances,
  focusCoords
}) {
  const mapContainerRef = useRef(null);
  const mapInstanceRef = useRef(null);
  const roadLinesRef = useRef([]);
  const markersRef = useRef({});
  const ambMarkersRef = useRef({});

  // 1. Initialize Leaflet Map once
  useEffect(() => {
    if (!mapContainerRef.current || mapInstanceRef.current) return;

    // Center on central Pune (Shivajinagar / Pune Station area)
    const map = L.map(mapContainerRef.current, {
      center: [18.5204, 73.8567],
      zoom: 12,
      minZoom: 10,
      maxZoom: 18,
      zoomControl: false,
      attributionControl: false
    });

    // Clean OpenStreetMap Tiles (Fast, high-detail, zero API key watermark)
    L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
      maxZoom: 19
    }).addTo(map);

    // Zoom control at top-right
    L.control.zoom({ position: 'topright' }).addTo(map);

    mapInstanceRef.current = map;

    return () => {
      map.remove();
      mapInstanceRef.current = null;
    };
  }, []);

  // 2. Fly to sector when focusCoords change
  useEffect(() => {
    if (!mapInstanceRef.current || !focusCoords) return;
    mapInstanceRef.current.flyTo([focusCoords.lat, focusCoords.lng], focusCoords.zoom || 13, {
      duration: 1.2,
      easeLinearity: 0.25
    });
  }, [focusCoords]);

  // 3. Draw / Update Road Polylines
  useEffect(() => {
    const map = mapInstanceRef.current;
    if (!map) return;

    // Clear old road lines
    roadLinesRef.current.forEach(line => map.removeLayer(line));
    roadLinesRef.current = [];

    PUNE_ROADS.forEach(road => {
      const u = PUNE_INTERSECTIONS.find(n => n.id === road.u);
      const v = PUNE_INTERSECTIONS.find(n => n.id === road.v);
      if (!u || !v) return;

      const isPreempted = preemptedCorridor.includes(road.u) && preemptedCorridor.includes(road.v);
      const isBlocked = blockedNodes.has(road.u) || blockedNodes.has(road.v);

      let color = '#4B5563';
      let weight = road.type === 'HIGHWAY' ? 4.5 : 3;
      let opacity = 0.7;
      let dashArray = null;

      if (road.type === 'HIGHWAY') {
        dashArray = '6, 6';
        color = '#374151';
        weight = 5;
      }

      if (isPreempted) {
        color = '#0D9488'; // Vibrant teal/cyan green wave corridor
        weight = 6;
        opacity = 0.95;
        dashArray = null;
      } else if (isBlocked) {
        color = '#DC2626'; // Red incident blocked
        weight = 5;
        opacity = 0.95;
      }

      const polyline = L.polyline([[u.lat, u.lng], [v.lat, v.lng]], {
        color,
        weight,
        opacity,
        dashArray,
        lineCap: 'round',
        lineJoin: 'round'
      }).addTo(map);

      polyline.bindTooltip(`<strong>${road.name}</strong><br>Speed: ${road.speedKmH} km/h • Cap: ${road.cap} vph`, {
        sticky: true,
        direction: 'top'
      });

      roadLinesRef.current.push(polyline);
    });
  }, [blockedNodes, preemptedCorridor]);

  // 4. Draw / Update Intersections Markers
  useEffect(() => {
    const map = mapInstanceRef.current;
    if (!map) return;

    // Clear old markers from map
    Object.values(markersRef.current).forEach(m => map.removeLayer(m));
    markersRef.current = {};

    PUNE_INTERSECTIONS.forEach(node => {
      const isSelected = selectedNodeId === node.id;
      const isCompromised = compromisedNodes.has(node.id);
      const isBlocked = blockedNodes.has(node.id);
      const isPreempted = preemptedCorridor.includes(node.id);

      let badgeColor = node.signal.nsGreen ? '#16A34A' : '#D97706';
      if (isCompromised) badgeColor = '#DC2626';
      if (isBlocked) badgeColor = '#991B1B';
      if (isPreempted) badgeColor = '#0D9488';

      const ringSize = isSelected ? 28 : 22;
      const ringBorder = isSelected ? '3px solid #111827' : `2.5px solid ${badgeColor}`;
      const glow = isCompromised
        ? 'box-shadow: 0 0 12px #DC2626;'
        : isSelected
        ? 'box-shadow: 0 0 10px rgba(0,0,0,0.5);'
        : 'box-shadow: 0 2px 6px rgba(0,0,0,0.25);';

      const customHtml = `
        <div style="position: relative; width: 30px; height: 30px; display: flex; align-items: center; justify-content: center; cursor: pointer;">
          <div style="position: absolute; width: ${ringSize}px; height: ${ringSize}px; border-radius: 50%; border: ${ringBorder}; background: #FFFFFF; ${glow}"></div>
          <div style="position: absolute; width: 8px; height: 8px; border-radius: 50%; background: ${badgeColor};"></div>
          <span style="position: absolute; top: -15px; font-family: monospace; font-size: 10px; font-weight: 800; color: #111827; text-shadow: 0 1px 2px #FFF, 0 -1px 2px #FFF, 1px 0 2px #FFF, -1px 0 2px #FFF;">${String(node.id).padStart(2, '0')}</span>
        </div>
      `;

      const icon = L.divIcon({
        className: 'custom-leaflet-pin',
        html: customHtml,
        iconSize: [30, 30],
        iconAnchor: [15, 15]
      });

      const marker = L.marker([node.lat, node.lng], { icon, zIndexOffset: isSelected ? 500 : 100 })
        .addTo(map)
        .on('click', () => onSelectNode(node.id));

      marker.bindTooltip(`<strong>${node.name}</strong><br><span style="font-size: 11px; color: #555;">Gateway: ${node.gateway} • Sector: ${node.sector}</span>`, {
        direction: 'top',
        offset: [0, -14],
        opacity: 0.95
      });

      markersRef.current[node.id] = marker;
    });
  }, [selectedNodeId, compromisedNodes, blockedNodes, preemptedCorridor, onSelectNode]);

  // 5. Draw / Update Ambulances
  useEffect(() => {
    const map = mapInstanceRef.current;
    if (!map) return;

    // Clear old ambulance markers
    Object.values(ambMarkersRef.current).forEach(m => map.removeLayer(m));
    ambMarkersRef.current = {};

    ambulances.forEach(amb => {
      const uNode = PUNE_INTERSECTIONS.find(n => n.id === amb.currentNode);
      const currentLat = amb.lat !== undefined ? amb.lat : (uNode ? uNode.lat : 18.5204);
      const currentLng = amb.lng !== undefined ? amb.lng : (uNode ? uNode.lng : 73.8567);

      const isBusy = amb.isEmergency || (amb.path && amb.path.length > 0);
      const bgColor = isBusy ? '#DC2626' : '#2563EB'; // Red flashing if priority dispatch, Vibrant Blue if active patrol!

      const ambHtml = `
        <div style="background: ${bgColor}; color: #FFFFFF; border: 2.5px solid #FFFFFF; border-radius: 50%; width: 28px; height: 28px; display: flex; align-items: center; justify-content: center; font-size: 10px; font-weight: 800; font-family: monospace; box-shadow: 0 4px 10px rgba(0,0,0,0.45); cursor: pointer;">
          108
        </div>
      `;

      const icon = L.divIcon({
        className: 'custom-ambulance-pin',
        html: ambHtml,
        iconSize: [28, 28],
        iconAnchor: [14, 14]
      });

      const marker = L.marker([currentLat, currentLng], { icon, zIndexOffset: isBusy ? 2000 : 1000 }).addTo(map);
      marker.bindTooltip(`🚑 <strong>${amb.name}</strong><br><span style="color: ${isBusy ? '#DC2626' : '#2563EB'}; font-weight: 700;">${amb.status}</span><br>Base: ${amb.hospital}`, { direction: 'top' });
      ambMarkersRef.current[amb.id] = marker;
    });
  }, [ambulances]);

  return (
    <div style={{ width: '100%', height: '100%', position: 'relative' }}>
      <div ref={mapContainerRef} style={{ width: '100%', height: '100%' }} />

      {/* Floating Minimalist GIS HUD Legend */}
      <div className="map-gis-legend">
        <div className="legend-title">PUNE MUNICIPAL GIS LAYERS</div>
        <div className="legend-items">
          <div className="legend-item"><span className="legend-dot dot-signal-green"></span> Signal Normal (Green Phase)</div>
          <div className="legend-item"><span className="legend-dot dot-signal-yellow"></span> Signal Hold (East-West)</div>
          <div className="legend-item"><span className="legend-dot dot-cyber-attack"></span> Cyber Compromised Node</div>
          <div className="legend-item"><span className="legend-line line-preempted"></span> 108 Preemption Corridor</div>
          <div className="legend-item"><span className="legend-dot dot-ambulance"></span> 108 EMS Fleet Unit</div>
        </div>
      </div>
    </div>
  );
}
