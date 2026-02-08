const express = require("express");
const mqtt = require("mqtt");
const path = require("path");
const { Client } = require("pg");
const http = require("http");
const { Server } = require("socket.io");

const app = express();
//server port
const port = 3000;
const server = http.createServer(app);
const io = new Server(server);

app.use(express.json());

const mqttClient = mqtt.connect("mqtt://localhost:1883"); //may need to be adjusted to RPi's IP

//connection to the database
const con = new Client({
  host: "localhost",
  username: "admin",
  port: 5432,
  password: "1234567890...",
  database: "pidData",
});
con.connect().then(() => console.log("Connected to Database"));

// MQTT setup
mqttClient.on("connect", () => {
  console.log("Backend connected to MQTT server");
  //subscribes the server to "performanceData" topic
  mqttClient.subscribe("performanceData");
});

// Middleware to parse JSON from frontend
app.use(express.json());
app.use(express.static("public")); // Serves your HTML file

// Send commands from Frontend to ESP32
app.post("/api/command", (req, res) => {
  const { kp, ki, kd, set } = req.body;
  const payload = JSON.stringify({
    kp: kp || null,
    ki: ki || null,
    kd: kd || null,
    set,
  });

  // 1. Wipe the database table
  const clearQuery = "TRUNCATE TABLE performancedata"; // Use TRUNCATE for a fast, total wipe

  con.query(clearQuery, (err) => {
    if (err) {
      console.error("Error clearing database:", err.message);
      return res.status(500).json({ error: "Failed to clear old data" });
    }

    console.log("Database table performancedata wiped.");

    // 2. Tell the frontend to clear the graph
    io.emit("clearGraph");

    // 3. Publish the new PID values to the ESP32
    mqttClient.publish("commandData", payload, (mqttErr) => {
      if (mqttErr) return res.status(500).json({ error: "Failed to publish" });
      res.json({ status: "Database wiped and command sent!" });
    });
  });
});

// For receiving MQTT data
mqttClient.on("message", (topic, message) => {
  try {
    const data = JSON.parse(message.toString());

    if (topic === "performanceData") {
      const insertQuery = `
        INSERT INTO performanceData (riseTime, settlingTime, steadyStateError, overshoot, setpoint, actualValue) 
        VALUES ($1, $2, $3, $4, $5, $6)
      `;
      const values = [
        data.riseTime,
        data.settlingTime,
        data.steadyStateError,
        data.overshoot,
        data.setpoint,
        data.actualValue,
      ];

      con.query(insertQuery, values, (err, result) => {
        if (err) {
          console.error("Database Error:", err.message);
        } else {
          console.log("Performance data saved to DB");
        }
      });
      io.emit("realtimeData", data);
    }
  } catch (error) {
    console.error("Received message was not valid JSON");
  }
});

//serve static files
app.use(express.static(path.join(__dirname, "frontend")));

server.listen(port, () => {
  console.log(`Server and WebSockets listening on port ${port}`);
});
