const express = require("express");
const { default: mqtt } = require("mqtt");
const path = require("path");
const app = express();
const port = 3000;

app.use(express.json());

const mqttClient = mqtt.connect();
const mqttTopic = "esp32/sensor";

mqttClient.on("connect", () => {
  console.log("connected to MQTT broker");

  mqttClient.subscribe(mqttTopic, (err) => {
    if (!err) {
      console.log(`listening for data on topic: ${mqttTopic}`);
    }
  });
});

mqttClient.on("message", (topic, message) => {
  const msgStr = message.toString();
  console.log(`MQTT message received: ${msgStr}`);
});

//serve static files
app.use(express.static(path.join(__dirname, "frontend")));

app.listen(port, () => {
  console.log(`listening on port ${port}`);
});
