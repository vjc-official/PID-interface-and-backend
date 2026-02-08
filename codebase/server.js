const express = require("express");
const mqtt = require("mqtt");
const path = require("path");

const app = express();
//server port
const port = 3000;

app.use(express.json());
const client = mqtt.connect("mqtt://localhost:1883"); //may need to be adjusted to RPi's IP

// MQTT setup
client.on("connect", () => {
  console.log("Backend connected to MQTT server");
  //subscribes the server to "performanceData" topic
  client.subscribe("performanceData");
});

// Middleware to parse JSON from frontend
app.use(express.json());
app.use(express.static('public')); // Serves your HTML file

app.post('/api/command', (req, res) => {
    const { kp, ki, kd, set } = req.body;
    
    const payload = JSON.stringify({ kp, ki, kd, set });
    
    client.publish("commandData", payload, (err) => {
        if (err) {
            return res.status(500).json({ error: "Failed to publish" });
        }
        res.json({ status: "Command sent!" });
    });
});

// For receiving MQTT data
client.on("message", (topic, message) => {

  try {
    const data = JSON.parse(message);
      console.log(`Rise Time: ${data.riseTime}, Settling Time: ${data.settlingTime}, Steady State Error: ${data.steadyStateError}, Overshoot: ${data.overshoot}, Setpoint: ${data.setpoint}, Actual Value: ${data.actualValue}`);
  } catch (error) {
    console.error("Received message was not valid JSON");
  }
});

//serve static files
app.use(express.static(path.join(__dirname, "frontend")));

app.listen(port, () => {
  console.log(`listening on port ${port}`);
});
