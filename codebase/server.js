const express = require("express");
const mqtt = require("mqtt");
const path = require("path");
const app = express();
//server port
const port = 3000;

app.use(express.json());

const client = mqtt.connect("mqtt://localhost:1883");

client.on("connect", () => {
  console.log("Backend connected to MQTT server");
  //subscribes the server to "performanceData" topic
  client.subscribe("performanceData");
});

client.on("message", (topic, message) => {
  const payload = message.toString(); //Step 1: Buffer (received data) -> convert to string

  try {
    const data = JSON.parse(payload);
  } catch (error) {
    console.error("Received message was not valid JSON");
  }

  console.log(data.riseTime);
});

//serve static files
app.use(express.static(path.join(__dirname, "frontend")));

app.listen(port, () => {
  console.log(`listening on port ${port}`);
});
