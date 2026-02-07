const express = require('express');
const cors = require('cors');
const bodyParser = require('body-parser');
const rateLimit = require('express-rate-limit');
const Joi = require('joi');

const app = express();
const PORT = 5000;


app.use(cors());
app.use(bodyParser.json());


const limiter = rateLimit({
    windowMs: 1 * 60 * 1000,
    max: 60,
    message: "Too many requests, please slow down."
});
app.use('/data', limiter);


const sensorSchema = Joi.object({
    temp: Joi.number().min(-10).max(80).required(),
    hum: Joi.number().min(0).max(100).required(),
    risk_score: Joi.number().integer().min(0).max(100).required(),
    status_msg: Joi.string().max(50).required(),
    alert_level: Joi.number().valid(0, 1, 2).required(),
    free_heap: Joi.number().integer().optional()
});


let systemState = {
    latest: {},
    history: [],
    alerts: []
};


app.post('/data', (req, res) => {

    const { error, value } = sensorSchema.validate(req.body);

    if (error) {
        console.error("❌ Invalid Data Rejected:", error.details[0].message);
        return res.status(400).json({ error: "Invalid Payload" });
    }


    const timestamp = new Date().toISOString();
    const dataPoint = { ...value, timestamp };


    systemState.latest = dataPoint;


    systemState.history.push(dataPoint);
    if (systemState.history.length > 50) systemState.history.shift();


    if (value.alert_level === 2) {
        console.error(`🚨 CRITICAL ALERT: ${value.status_msg} (Score: ${value.risk_score})`);
        systemState.alerts.push({ msg: value.status_msg, time: timestamp });
    } else {
        console.log(`✅ [${timestamp}] T:${value.temp}°C H:${value.hum}% | Status: ${value.status_msg}`);
    }

    res.status(200).send("Data Processed");
});


app.get('/data', (req, res) => {
    res.json(systemState);
});


app.listen(PORT, '0.0.0.0', () => {
    console.log(`🛡️  Secure IoT Backend running on Port ${PORT}`);
});