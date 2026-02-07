import React, { useState, useEffect, useRef } from 'react';
import { AreaChart, Area, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer, BarChart, Bar, Legend } from 'recharts';
import { Activity, Droplets, Thermometer, Zap, Download, Pause, Play, Server, Wifi, AlertTriangle, Clock, Database } from 'lucide-react';
import './App.css';

// --- SUB-COMPONENT: MONITORING DASHBOARD (Your existing dashboard) ---
const MonitoringView = ({ data, stats, isPaused, setIsPaused, downloadCSV, temp, hum, risk, dewPoint, heatIndex }) => (
  <>
    <div className="top-bar">
      <div>
        <h2 style={{margin:0}}>Environmental Overview</h2>
        <div style={{color: '#94a3b8', fontSize: '0.9rem'}}>Live Telemetry • 1s Interval</div>
      </div>
      
      <div className="controls">
        <div className={`status-indicator ${data.latest.alert_level === 2 ? 'critical' : ''}`}>
          <div className={`dot ${data.latest.alert_level === 2 ? 'red' : 'green'}`}></div>
          {data.latest.status_msg || "Connecting..."}
        </div>
        <button className="btn" onClick={() => setIsPaused(!isPaused)}>
          {isPaused ? <Play size={16}/> : <Pause size={16}/>}
        </button>
        <button className="btn btn-primary" onClick={downloadCSV}>
          <Download size={16}/> Export Data
        </button>
      </div>
    </div>

    {/* METRICS GRID */}
    <div className="metrics-grid">
      <div className="metric-card">
        <div className="metric-header">
          <span>Temperature</span>
          <Thermometer size={18} color="#ef4444"/>
        </div>
        <div className="metric-value">{temp}<span className="metric-unit">°C</span></div>
        <div className="metric-footer">Range: {stats.minTemp}° - {stats.maxTemp}°</div>
      </div>

      <div className="metric-card">
        <div className="metric-header">
          <span>Humidity</span>
          <Droplets size={18} color="#3b82f6"/>
        </div>
        <div className="metric-value">{hum}<span className="metric-unit">%</span></div>
        <div className="metric-footer">Dew Point: {dewPoint}°C</div>
      </div>

      <div className="metric-card">
        <div className="metric-header">
          <span>Heat Index</span>
          <Zap size={18} color="#f59e0b"/>
        </div>
        <div className="metric-value">{heatIndex}<span className="metric-unit">°C</span></div>
        <div className="metric-footer">Perceived Temp</div>
      </div>

      <div className="metric-card" style={{borderColor: risk > 50 ? '#ef4444' : 'rgba(255,255,255,0.1)'}}>
        <div className="metric-header">
          <span>Risk Index</span>
          <Activity size={18} color={risk > 50 ? '#ef4444' : '#10b981'}/>
        </div>
        <div className="metric-value">{risk}<span className="metric-unit">/100</span></div>
        <div className="metric-footer">AI Assessment</div>
      </div>
    </div>

    {/* CHARTS */}
    <div className="chart-card">
      <div className="chart-header">
        <h3>Real-Time Trends</h3>
      </div>
      <ResponsiveContainer width="100%" height="85%">
        <AreaChart data={data.history}>
          <defs>
            <linearGradient id="colorTemp" x1="0" y1="0" x2="0" y2="1">
              <stop offset="5%" stopColor="#ef4444" stopOpacity={0.3}/>
              <stop offset="95%" stopColor="#ef4444" stopOpacity={0}/>
            </linearGradient>
            <linearGradient id="colorHum" x1="0" y1="0" x2="0" y2="1">
              <stop offset="5%" stopColor="#3b82f6" stopOpacity={0.3}/>
              <stop offset="95%" stopColor="#3b82f6" stopOpacity={0}/>
            </linearGradient>
          </defs>
          <CartesianGrid strokeDasharray="3 3" stroke="#334155" vertical={false} />
          <XAxis dataKey="timestamp" hide={true} />
          <YAxis domain={['auto', 'auto']} stroke="#94a3b8" fontSize={12} tickLine={false} axisLine={false}/>
          <Tooltip contentStyle={{ backgroundColor: '#1e293b', borderColor: '#334155', color: '#f8fafc' }} itemStyle={{ color: '#f8fafc' }} />
          <Area type="monotone" dataKey="temp" stroke="#ef4444" strokeWidth={2} fillOpacity={1} fill="url(#colorTemp)" />
          <Area type="monotone" dataKey="hum" stroke="#3b82f6" strokeWidth={2} fillOpacity={1} fill="url(#colorHum)" />
        </AreaChart>
      </ResponsiveContainer>
    </div>
  </>
);

// --- SUB-COMPONENT: DEVICES VIEW ---
const DevicesView = ({ latest }) => (
  <div className="view-container">
    <h2>Connected Hardware</h2>
    <div className="metrics-grid" style={{marginTop: '20px'}}>
      
      {/* ESP32 Card */}
      <div className="metric-card">
        <div className="metric-header">
          <span>ESP32 Main Controller</span>
          <Wifi size={18} color="#10b981"/>
        </div>
        <div className="stat-row" style={{marginTop: '15px'}}>
          <span>Status</span>
          <span style={{color: '#10b981', fontWeight: 'bold'}}>ONLINE</span>
        </div>
        <div className="stat-row">
          <span>IP Address</span>
          <span className="stat-value">192.168.0.106</span>
        </div>
        <div className="stat-row">
          <span>Memory (Free Heap)</span>
          <span className="stat-value">{latest.free_heap || 0} bytes</span>
        </div>
        <div className="stat-row">
          <span>Firmware Ver</span>
          <span className="stat-value">v1.2.0</span>
        </div>
      </div>

      {/* Sensor Card */}
      <div className="metric-card">
        <div className="metric-header">
          <span>DHT11 Sensor</span>
          <Activity size={18} color="#3b82f6"/>
        </div>
        <div className="stat-row" style={{marginTop: '15px'}}>
          <span>Protocol</span>
          <span className="stat-value">Single-Wire (GPIO 15)</span>
        </div>
        <div className="stat-row">
          <span>Polling Rate</span>
          <span className="stat-value">1000 ms</span>
        </div>
        <div className="stat-row">
          <span>Last Read</span>
          <span className="stat-value">Just now</span>
        </div>
      </div>

      {/* Backend Card */}
      <div className="metric-card">
        <div className="metric-header">
          <span>Edge Gateway (Node.js)</span>
          <Server size={18} color="#f59e0b"/>
        </div>
        <div className="stat-row" style={{marginTop: '15px'}}>
          <span>Port</span>
          <span className="stat-value">5000</span>
        </div>
        <div className="stat-row">
          <span>Rate Limit</span>
          <span className="stat-value">120 req/min</span>
        </div>
        <div className="stat-row">
          <span>Validation</span>
          <span className="stat-value">Active (Joi)</span>
        </div>
      </div>

    </div>
  </div>
);

// --- SUB-COMPONENT: ANALYTICS VIEW ---
const AnalyticsView = ({ history }) => (
  <div className="view-container">
    <h2>System Analytics</h2>
    <div className="chart-card" style={{marginTop: '20px'}}>
      <div className="chart-header">
        <h3>Risk Score Distribution</h3>
      </div>
      <ResponsiveContainer width="100%" height="85%">
        <BarChart data={history.slice(-20)}> {/* Last 20 points */}
          <CartesianGrid strokeDasharray="3 3" stroke="#334155" vertical={false} />
          <XAxis dataKey="timestamp" hide={true} />
          <YAxis stroke="#94a3b8" />
          <Tooltip cursor={{fill: 'rgba(255,255,255,0.05)'}} contentStyle={{ backgroundColor: '#1e293b', borderColor: '#334155', color: '#f8fafc' }} />
          <Legend />
          <Bar dataKey="risk_score" fill="#f59e0b" name="Risk Score" radius={[4, 4, 0, 0]} />
        </BarChart>
      </ResponsiveContainer>
    </div>

    <div className="metrics-grid" style={{marginTop: '20px'}}>
      <div className="metric-card">
        <div className="metric-header"><span>Uptime</span><Clock size={18}/></div>
        <div className="metric-value">99.9<span className="metric-unit">%</span></div>
      </div>
      <div className="metric-card">
        <div className="metric-header"><span>Data Points Logged</span><Database size={18}/></div>
        <div className="metric-value">{history.length}<span className="metric-unit">pts</span></div>
      </div>
      <div className="metric-card">
        <div className="metric-header"><span>Anomalies Detected</span><AlertTriangle size={18}/></div>
        <div className="metric-value">0<span className="metric-unit">events</span></div>
      </div>
    </div>
  </div>
);

// --- MAIN APP ---
function App() {
  const [data, setData] = useState({ latest: {}, history: [] });
  const [stats, setStats] = useState({ minTemp: 999, maxTemp: -999, avgTemp: 0 });
  const [isPaused, setIsPaused] = useState(false);
  const [activeTab, setActiveTab] = useState('monitoring'); // <--- NEW STATE FOR NAVIGATION
  const historyRef = useRef([]); 

  const calculateDewPoint = (T, RH) => {
    return (T - (14.55 + 0.114 * T) * (1 - (0.01 * RH)) - Math.pow(((2.5 + 0.007 * T) * (1 - (0.01 * RH))), 3) - (15.9 + 0.117 * T) * Math.pow((1 - (0.01 * RH)), 14)).toFixed(1);
  };

  const calculateHeatIndex = (T, RH) => {
    return (T + 0.5555 * ((6.11 * Math.exp(5417.7530 * ((1/273.16) - (1/(273.15 + T))))) - 10)).toFixed(1);
  };

  useEffect(() => {
    const fetchData = async () => {
      if (isPaused) return;
      try {
        const response = await fetch('http://localhost:5000/data');
        const result = await response.json();
        
        if (result.latest.temp) {
          setStats(prev => ({
            minTemp: Math.min(prev.minTemp, result.latest.temp),
            maxTemp: Math.max(prev.maxTemp, result.latest.temp),
            avgTemp: prev.avgTemp === 0 ? result.latest.temp : ((prev.avgTemp + result.latest.temp) / 2).toFixed(1)
          }));
        }

        historyRef.current = result.history;
        setData(result);
      } catch (error) {
        console.error("Connection Error");
      }
    };

    fetchData(); 
    const interval = setInterval(fetchData, 1000); 
    return () => clearInterval(interval);
  }, [isPaused]); 

  const downloadCSV = () => {
    const headers = "Time,Temperature,Humidity,RiskScore\n";
    const rows = historyRef.current.map(row => 
      `${row.timestamp},${row.temp},${row.hum},${row.risk_score}`
    ).join("\n");
    const blob = new Blob([headers + rows], { type: 'text/csv' });
    const url = window.URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = `iot_session_${new Date().getTime()}.csv`;
    a.click();
  };

  const temp = data.latest.temp || 0;
  const hum = data.latest.hum || 0;
  const risk = data.latest.risk_score || 0;
  const dewPoint = calculateDewPoint(temp, hum);
  const heatIndex = calculateHeatIndex(temp, hum);

  // VIEW RENDERER
  const renderContent = () => {
    switch(activeTab) {
      case 'monitoring':
        return <MonitoringView data={data} stats={stats} isPaused={isPaused} setIsPaused={setIsPaused} downloadCSV={downloadCSV} temp={temp} hum={hum} risk={risk} dewPoint={dewPoint} heatIndex={heatIndex} />;
      case 'devices':
        return <DevicesView latest={data.latest} />;
      case 'analytics':
        return <AnalyticsView history={data.history} />;
      default:
        return <MonitoringView />;
    }
  };

  return (
    <div className="app-container">
      {/* SIDEBAR */}
      <div className="sidebar">
        <div className="brand">
          <Activity size={28} /> AURA PRO
        </div>
        
        {/* Navigation Buttons with Click Handlers */}
        <div 
          className={`nav-item ${activeTab === 'monitoring' ? 'active' : ''}`} 
          onClick={() => setActiveTab('monitoring')}
        >
          <Activity size={20}/> Monitoring
        </div>
        
        <div 
          className={`nav-item ${activeTab === 'devices' ? 'active' : ''}`} 
          onClick={() => setActiveTab('devices')}
        >
          <Server size={20}/> Devices
        </div>
        
        <div 
          className={`nav-item ${activeTab === 'analytics' ? 'active' : ''}`} 
          onClick={() => setActiveTab('analytics')}
        >
          <Zap size={20}/> Analytics
        </div>

        <div style={{marginTop: 'auto', fontSize: '0.8rem', color: '#64748b'}}>
          v2.0.1 Stable
        </div>
      </div>

      {/* MAIN CONTENT AREA */}
      <div className="main-content">
        {renderContent()}
      </div>
    </div>
  );
}

export default App;