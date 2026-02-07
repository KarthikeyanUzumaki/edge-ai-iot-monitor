import React, { useState, useEffect } from 'react';
import axios from 'axios';
import './App.css';

function App() {
  const [data, setData] = useState(null);
  const [lastUpdated, setLastUpdated] = useState(null);

  // Function to fetch data from our Node.js Backend
  const fetchData = async () => {
    try {
      // Connects to your local Node.js server
      const response = await axios.get('http://localhost:5000/data');
      setData(response.data);
      setLastUpdated(new Date().toLocaleTimeString());
    } catch (error) {
      console.error("Error fetching data:", error);
    }
  };

  // Auto-refresh every 2 seconds
  useEffect(() => {
    fetchData(); // Fetch immediately on load
    const interval = setInterval(fetchData, 2000);
    return () => clearInterval(interval);
  }, []);

  return (
    <div className="App">
      <header className="App-header">
        <h1>⚡ IoT Live Monitor</h1>
        <p>Status: {data ? "🟢 Online" : "🔴 Connecting..."}</p>
        <p className="timestamp">Last Updated: {lastUpdated}</p>
      </header>

      {data && (
        <div className="dashboard-grid">
          {/* Temperature Card */}
          <div className="card temp-card">
            <h2>🌡️ Temperature</h2>
            <div className="value">{data.temp.toFixed(1)}°C</div>
          </div>

          {/* Humidity Card */}
          <div className="card hum-card">
            <h2>💧 Humidity</h2>
            <div className="value">{data.hum.toFixed(1)}%</div>
          </div>

          {/* Motion Card */}
          <div className="card motion-card">
            <h2>🚀 Motion (Accel)</h2>
            <div className="value-small">
              X: {data.accel_x.toFixed(2)} <br/>
              Y: {data.accel_y.toFixed(2)}
            </div>
          </div>
        </div>
      )}
    </div>
  );
}

export default App;