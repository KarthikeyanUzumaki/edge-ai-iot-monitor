import React, { useState, useEffect } from 'react';
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer } from 'recharts';
import { Activity, Thermometer, Droplets, Cpu, AlertTriangle, CheckCircle } from 'lucide-react';
import './App.css';

function App() {
  const [data, setData] = useState(null);
  const [loading, setLoading] = useState(true);

  const fetchData = async () => {
    try {
      const response = await fetch('http://localhost:5000/data');
      const result = await response.json();
      setData(result);
      setLoading(false);
    } catch (error) {
      console.error('Error fetching data:', error);
    }
  };

  useEffect(() => {
    fetchData(); // Initial fetch
    const interval = setInterval(fetchData, 2000); // Poll every 2 seconds
    return () => clearInterval(interval);
  }, []);

  if (loading) return <div className="loading-screen">Initialize System...</div>;

  const { latest, history } = data;
  const isCritical = latest.alert_level === 2;
  const isWarning = latest.alert_level === 1;

  // Status Badge Logic
  const getStatusColor = () => {
    if (isCritical) return 'status-critical';
    if (isWarning) return 'status-warning';
    return 'status-normal';
  };

  return (
    <div className="dashboard">
      {/* HEADER */}
      <header className="header">
        <div className="logo-section">
          <Activity className="icon-logo" />
          <h1>AURA <span className="subtitle">PRO</span></h1>
        </div>
        <div className={`status-badge ${getStatusColor()}`}>
          {isCritical ? <AlertTriangle size={16} /> : <CheckCircle size={16} />}
          <span>{latest.status_msg || "SYSTEM ONLINE"}</span>
        </div>
      </header>

      {/* METRIC CARDS */}
      <div className="grid-container">
        
        {/* TEMPERATURE CARD */}
        <div className="card">
          <div className="card-header">
            <Thermometer className="icon-temp" />
            <h3>Temperature</h3>
          </div>
          <div className="value-display">
            {latest.temp}°C
          </div>
          <div className="risk-score">
            Risk Score: {latest.risk_score}/100
          </div>
        </div>

        {/* HUMIDITY CARD */}
        <div className="card">
          <div className="card-header">
            <Droplets className="icon-hum" />
            <h3>Humidity</h3>
          </div>
          <div className="value-display">
            {latest.hum}%
          </div>
          <div className="sub-text">Relative Humidity</div>
        </div>

        {/* SYSTEM HEALTH CARD */}
        <div className="card">
          <div className="card-header">
            <Cpu className="icon-sys" />
            <h3>Device Health</h3>
          </div>
          <div className="stat-row">
            <span>Free Heap:</span>
            <span className="stat-value">{latest.free_heap} B</span>
          </div>
          <div className="stat-row">
            <span>Uptime:</span>
            <span className="stat-value">Active</span>
          </div>
        </div>
      </div>

      {/* CHART SECTION */}
      <div className="chart-section">
        <h3>Live Environmental Trends</h3>
        <div className="chart-container">
          <ResponsiveContainer width="100%" height="100%">
            <LineChart data={history}>
              <CartesianGrid strokeDasharray="3 3" stroke="#333" />
              <XAxis dataKey="timestamp" hide={true} />
              <YAxis domain={['auto', 'auto']} stroke="#888" />
              <Tooltip 
                contentStyle={{ backgroundColor: '#1f2937', border: 'none', color: '#fff' }} 
              />
              <Line 
                type="monotone" 
                dataKey="temp" 
                stroke="#ef4444" 
                strokeWidth={3} 
                dot={false} 
                animationDuration={500}
              />
              <Line 
                type="monotone" 
                dataKey="hum" 
                stroke="#3b82f6" 
                strokeWidth={3} 
                dot={false} 
                animationDuration={500}
              />
            </LineChart>
          </ResponsiveContainer>
        </div>
        <div className="legend">
          <span className="legend-item"><span className="dot red"></span> Temperature</span>
          <span className="legend-item"><span className="dot blue"></span> Humidity</span>
        </div>
      </div>
    </div>
  );
}

export default App;