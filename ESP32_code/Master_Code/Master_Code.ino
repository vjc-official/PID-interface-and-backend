#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <math.h>

// --- SETTINGS ---
const char* ssid     = "Converge 4G (HUAWEI-2.4G-3Rhm)";
const char* password = "Tan0ngM0KayTatay...";
const char* mqtt_server = "192.168.100.38";
const int mqtt_port    = 1883;
#define MPU_ADDR 0x68
#define IN1 26
#define IN2 27

// --- GLOBAL PID VARIABLES ---
float Kp = 15.0;
float Ki = 0.5;
float Kd = 3.5;
float setpoint = 70.0;

// --- PID STATE ---
float roll = 0, integral = 0, previous_error = 0;
unsigned long timer;

// --- ANALYTICS ---
unsigned long startTime = 0;
float maxRoll = 0;
bool systemStarted = false;
bool riseTimeCaptured = false;
bool settlingTimeCaptured = false;
float riseTime = 0, settlingTime = 0, overshoot = 0, steadyStateError = 0;
unsigned long settleStart = 0;

WiFiClient espClient;
PubSubClient client(espClient);

// --- 1. MQTT CALLBACK (Receiving from Frontend) ---
void callback(char* topic, byte* payload, unsigned int length) {
    StaticJsonDocument<200> doc;
    deserializeJson(doc, payload, length);

    // Update global PID values
    Kp = doc["kp"];
    Ki = doc["ki"];
    Kd = doc["kd"];
    setpoint = doc["set"];

    // Reset analytics for the new tuning run
    systemStarted = false;
    riseTimeCaptured = false;
    settlingTimeCaptured = false;
    riseTime = 0; settlingTime = 0; overshoot = 0; maxRoll = 0;
    integral = 0; // Clear integral windup for new run
    
    Serial.println("New PID Parameters Applied & Analytics Reset");
}

void setup() {
    Serial.begin(115200);
    
    // Motor Pins
    ledcAttach(IN1, 5000, 8);
    ledcAttach(IN2, 5000, 8);

    // MPU6050
    Wire.begin(21, 22);
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x6B);
    Wire.write(0);
    Wire.endTransmission(true);

    // WiFi & MQTT
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);

    timer = millis();
}

void loop() {
    if (!client.connected()) reconnect();
    client.loop();

    // --- SENSOR READING & COMPLEMENTARY FILTER ---
    int16_t ax, ay, az, gx;
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_ADDR, 14, true);
    ax = Wire.read() << 8 | Wire.read();
    ay = Wire.read() << 8 | Wire.read();
    az = Wire.read() << 8 | Wire.read();
    Wire.read(); Wire.read();
    gx = Wire.read() << 8 | Wire.read();

    float dt = (millis() - timer) / 1000.0;
    if (dt <= 0) dt = 0.01;
    timer = millis();

    float roll_acc = atan2(ay/16384.0, az/16384.0) * 180.0 / PI;
    roll = 0.99 * (roll + (gx/131.0) * dt) + 0.01 * roll_acc;

    // --- PID CALCULATION ---
    float error = setpoint - roll;
    integral += error * dt;
    integral = constrain(integral, -50, 50);
    float derivative = (error - previous_error) / dt;
    float output = Kp * error + Ki * integral + Kd * derivative;
    previous_error = error;

    // --- ANALYTICS LOGIC ---
    if (!systemStarted && abs(error) > 2) { systemStarted = true; startTime = millis(); }
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
            if (!settlingTimeCaptured && millis() - settleStart >= 500) {
                settlingTime = (settleStart - startTime) / 1000.0;
                settlingTimeCaptured = true;
            }
        } else { settleStart = 0; }
        steadyStateError = abs(error);
    }

    // --- MOTOR CONTROL ---
    int duty = constrain(abs(output), 0, 255);
    if (duty > 0 && duty < 90) duty = 90;
    if (output > 0) { ledcWrite(IN1, duty); ledcWrite(IN2, 0); }
    else { ledcWrite(IN1, 0); ledcWrite(IN2, duty); }

    // --- PUBLISH DATA EVERY 100ms (High frequency for Real-time Graph) ---
    static unsigned long lastMsg = 0;
    if (millis() - lastMsg > 100) { 
        lastMsg = millis();
        StaticJsonDocument<512> outDoc;
        outDoc["kp"] = Kp;
        outDoc["ki"] = Ki;
        outDoc["kd"] = Kd;
        outDoc["riseTime"] = riseTime;
        outDoc["settlingTime"] = settlingTime;
        outDoc["steadyStateError"] = steadyStateError;
        outDoc["overshoot"] = overshoot;
        outDoc["setpoint"] = setpoint;
        outDoc["actualValue"] = roll;

        char buffer[256];
        serializeJson(outDoc, buffer);
        client.publish("performanceData", buffer);
    }
}

void reconnect() {
    while (!client.connected()) {
        String clientId = "ESP32_PID_" + String(random(0xffff), HEX);
        if (client.connect(clientId.c_str())) {
            client.subscribe("commandData");
        } else { delay(5000); }
    }
}