#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Replace with your network credentials
const char* ssid     = "GlobeAtHome_81223_2.4";
const char* password = "PWkatb2E";

// MQTT Broker settings
const char* mqtt_server = "192.168.254.118"; // Replace with your Broker's Local IP
const int mqtt_port    = 1883;            // Default MQTT port

WiFiClient espClient;
PubSubClient client(espClient);

void connectToWiFi() {
Serial.begin(115200);

  // Start the connection process
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  // Loop until connected
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Connection successful
  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// --- HANDLING INCOMING COMMANDS ---
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("]");

  // 1. Create a buffer to hold the incoming data
  StaticJsonDocument<200> doc;

  // 2. Deserialize (Parse) the JSON string
  DeserializationError error = deserializeJson(doc, payload, length);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.f_str());
    return;
  }

  // 3. Extract values (using the keys defined in Node.js)
  int kp_mqtt = doc["kp"];
  int ki_mqtt = doc["ki"];
  int kd_mqtt = doc["kd"];
  int setAngle_mqtt = doc["set"];

  // 4. Act on the data
  //TUNING VALUES TO BE CHANGED HERE
  Serial.print("kp: "); Serial.println(kp_mqtt);
  Serial.print("ki: "); Serial.println(ki_mqtt);
  Serial.print("kd: "); Serial.println(kd_mqtt);
  Serial.print("set: "); Serial.println(setAngle_mqtt);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32_Client_" + String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      
      // SUBSCRIBE to commandData topic here
      client.subscribe("commandData");
      Serial.println("Subscribed to commandData");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      delay(5000);
    }
  }
}

void setup() {
  connectToWiFi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

 // --- PUBLISHING JSON PERFORMANCE DATA ---
  static unsigned long lastMsg = 0;
  if (millis() - lastMsg > 1000) { 
    lastMsg = millis();

    // 1. Create the JSON document
    StaticJsonDocument<512> doc;
    
    // 2. Add data to the object
    doc["riseTime"] = 0;
    doc["settlingTime"] = 1;
    doc["steadyStateError"] = 2; // Signal strength in dBm
    doc["overshoot"] = 3; // System health check
    doc["setpoint"] = 4;
    doc["actualValue"] = 5;

    // 3. Serialize JSON to a buffer
    char buffer[256];
    serializeJson(doc, buffer);

    // 4. Publish to performanceData
    Serial.print("Publishing JSON: ");
    Serial.println(buffer);
    client.publish("performanceData", buffer);
  }
}
