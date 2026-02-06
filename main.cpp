// TO ADD
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

/*
  Replace the SSID and Password according to your wifi
*/
const char *ssid = "<YOUR_WIFI_SSID_HERE>";
const char *password = "<YOUR_WIFI_SSID_PASSWORD_HERE>";

// Your MQTT broker ID
const char *mqttBroker = "192.168.100.22";
const int mqttPort = 1883;
// MQTT topics
const char *publishTopic = "sensorReadings";
const char *subscribeTopic = "commands";

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;
const int READ_CYCLE_TIME = 1000;

//TO ADD
// Connect to Wifi
void setup_wifi()
{
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

//TO ADD
// Callback function whenever an MQTT message is received
void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String message;
  for (int i = 0; i < length; i++)
  {
    Serial.print(message += (char)payload[i]);
  }
  Serial.println();
}

//TO ADD
void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");

    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);

    // Attempt to connect
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      // Subscribe to topic
      // client.subscribe(subscribeTopic);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// TO ADD IN SETUP
void setup()
{

  // Setup the wifi
  setup_wifi();
  client.setServer(mqttBroker, mqttPort);
  client.setCallback(callback);
}

// TO ADD IN LOOP
void loop()
{
  // Listen for mqtt message and reconnect if disconnected
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  // publish BME280 sensor readings periodically
  unsigned long now = millis();
  if (now - lastMsg > READ_CYCLE_TIME)
  {
    lastMsg = now;

    //  Publish MQTT messages
    char buffer[256];
    StaticJsonDocument<96> doc;
    doc["setPoint"] = setPoint;
    doc["riseTime"] = riseTime;
    doc["overshoot"] = overshoot;
    doc["settlingTime"] = settlingTime;
    doc["steadyStateError"] = steadyStateError;
    size_t n = serializeJson(doc, buffer);
    client.publish(publishTopic, buffer, n);
  }
}