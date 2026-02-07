const express = require('express');
const cors = require('cors');
const bodyParser = require('body-parser');

const app = express();
const PORT = 5000; // ESP32 sends data to port 5000

// Middleware
app.use(cors()); // Allow React to access this
app.use(bodyParser.json()); // Parse JSON data

// Storage for the latest sensor reading
let latestData = {
    temp: 0,
    hum: 0,
    accel_x: 0,
    accel_y: 0,
    timestamp: null
};

// 1. ESP32 sends data here
app.post('/data', (req, res) => {
    const incomingData = req.body;
    
    console.log("📥 RECEIVED:", incomingData);

    // Update the latest data
    latestData = {
        ...incomingData,
        timestamp: new Date().toISOString()
    };
    
    res.sendStatus(200);
});

// 2. React Dashboard gets data from here
app.get('/data', (req, res) => {
    res.json(latestData);
});

// Start the server
app.listen(PORT, '0.0.0.0', () => {
    console.log(`✅ Server running on Port ${PORT}`);
    console.log(`   Waiting for ESP32 data...`);
});