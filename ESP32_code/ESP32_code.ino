#include <Wire.h>
#include <math.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define MPU_ADDR 0x68

// ===== MOTOR PINS =====
#define IN1 26
#define IN2 27

// ===== PWM SETTINGS (ESP32 CORE 3.x) =====
const int freq = 5000;
const int resolution = 8;

// ===== WIFI & MQTT CONFIG =====
const char *ssid = "GlobeAtHome_81223_2.4"; //wifi name of common network
const char *password = "PWKatb2E"; //password of common network
const char *mqttBroker = "192.168.254.113"; //change to IP of server device
const int mqttPort = 1883; 
const char *publishTopic = "performanceData";
const char *subscribeTopic = "commandData";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
char msg[50];
int value = 0;

// ===== PID PARAMETERS =====
float Kp = 15.0;
float Ki = 0.5;
float Kd = 3.5;

float roll = 0;
float setpoint = 70.0;
float integral = 0;
float previous_error = 0;
unsigned long timer;
const int READ_CYCLE_TIME = 1000;

// ===== ANALYTICS =====
unsigned long startTime = 0;
float maxRoll = 0;
bool systemStarted = false;
bool riseTimeCaptured = false;
bool settlingTimeCaptured = false;

float riseTime = 0;
float settlingTime = 0;
float overshoot = 0;
float steadyStateError = 0;
unsigned long settleStart = 0;


// CONNECT ESP32 TO COMMON WIFI NETWORK
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid,password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("pidPropeller")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("commandData");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  // PWM Attach
  ledcAttach(IN1, freq, resolution);
  ledcAttach(IN2, freq, resolution);

  // MPU6050 Init
  Wire.begin(21, 22);
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  timer = millis();
  Serial.println("Roll,Setpoint,RiseTime,Overshoot,SettlingTime,SteadyStateError");

  initWiFi();
  client.setServer(mqttBroker, mqttPort);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();  // Corrected from 'Client.loop'

  // --- SENSOR READING & PID ---
  int16_t ax, ay, az, gx_raw;
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 14, true);

  ax = Wire.read() << 8 | Wire.read();
  ay = Wire.read() << 8 | Wire.read();
  az = Wire.read() << 8 | Wire.read();
  Wire.read();
  Wire.read();  // skip temp
  gx_raw = Wire.read() << 8 | Wire.read();

  float ax_g = ax / 16384.0;
  float ay_g = ay / 16384.0;
  float az_g = az / 16384.0;
  float gx_dps = gx_raw / 131.0;

  float roll_acc = atan2(ay_g, az_g) * 180.0 / PI;

  unsigned long now = millis();
  float dt = (now - timer) / 1000.0;
  if (dt <= 0) dt = 0.01;
  timer = now;

  // Complementary Filter
  roll = 0.99 * (roll + gx_dps * dt) + 0.01 * roll_acc;

  // PID Calculation
  float error = setpoint - roll;
  integral += error * dt;
  integral = constrain(integral, -50, 50);
  float derivative = (error - previous_error) / dt;
  float output = Kp * error + Ki * integral + Kd * derivative;
  previous_error = error;

  // --- ANALYTICS LOGIC ---
  if (!systemStarted && abs(error) > 2) {
    systemStarted = true;
    startTime = millis();
  }

  if (systemStarted) {
    if (!riseTimeCaptured && roll >= setpoint * 0.9) {
      riseTime = (millis() - startTime) / 1000.0;
      riseTimeCaptured = true;
    }
    if (roll > maxRoll) maxRoll = roll;
    if (maxRoll > setpoint) overshoot = ((maxRoll - setpoint) / setpoint) * 100.0;

    float settleBand = setpoint * 0.05;
    if (abs(error) < settleBand) {
      if (settleStart == 0) settleStart = millis();
      if (!settlingTimeCaptured && (millis() - settleStart >= 500)) {
        settlingTime = (settleStart - startTime) / 1000.0;
        settlingTimeCaptured = true;
      }
    } else {
      settleStart = 0;
    }
    steadyStateError = abs(error);
  }

  // --- MOTOR CONTROL ---
  int duty = constrain(abs((int)output), 0, 255);
  if (duty > 0 && duty < 90) duty = 90;

  if (output > 0) {
    ledcWrite(IN1, duty);
    ledcWrite(IN2, 0);
  } else {
    ledcWrite(IN1, 0);
    ledcWrite(IN2, duty);
  }

  // Serial Plotter output
  Serial.print(roll);
  Serial.print(",");
  Serial.print(setpoint);
  Serial.print(",");
  Serial.print(riseTime);
  Serial.print(",");
  Serial.print(overshoot);
  Serial.print(",");
  Serial.print(settlingTime);
  Serial.print(",");
  Serial.println(steadyStateError);

  delay(10);
}
