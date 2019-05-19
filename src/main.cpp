/* Necessary libraries */
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include <AsyncMqttClient.h>

#include "config.h"

#define SINGLE_LED D5
#define RED D1
#define GREEN D2
#define BLUE D3
#define LDR A0

void rgbColor(int, int, int);
float ldrVoltage();
void onMqttConnect(bool);
void onMqttMessage(char *, char *, AsyncMqttClientMessageProperties, size_t, size_t, size_t);


int start = 0;

// MQTT username: node-mcu
// MQTT password: cgNEYFjS9xZeJwMc
ESP8266WebServer server(80);
IPAddress mqttServer(207, 154, 249, 118);
const int port = 1883;
AsyncMqttClient mqttClient;

void setup() {
    /* Pin modes */
    pinMode(SINGLE_LED, OUTPUT);
    pinMode(RED, OUTPUT);
    pinMode(GREEN, OUTPUT);
    pinMode(BLUE, OUTPUT);
    pinMode(LDR, INPUT);

    rgbColor(0, 0, 0);
    delay(1000);
    Serial.begin(115200);

    /* Establish connection */
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifi_ssid, wifi_password);
    
    /* SmartConfig code */
    
    // WiFi.setAutoReconnect(true);
    // WiFi.beginSmartConfig();
    // Serial.print("[SmartConfig] Waiting for credentials...");
    // while (!WiFi.smartConfigDone()) {
    //     delay(500);
    //     Serial.print(".");
    // }
    // Serial.println("");
    // Serial.println("[SmartConfig] SmartConfig done.");
    
    Serial.print("[WiFi] Connecting...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.printf("\n[WiFi] Successfully connected to %s, with IP address %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
    /* Connect to MQTT server */
    if (WiFi.isConnected()) {
        mqttClient.onConnect(onMqttConnect);
        mqttClient.onMessage(onMqttMessage);
        mqttClient.setCleanSession(true);
        mqttClient.setServer(mqttServer, port);
        mqttClient.setCredentials(mqtt_user, mqtt_password);
        /* Set the Last Will & Testament (LWT) */
        mqttClient.setWill("/lwt", 0, true, "NodeMCU has disconnected.");
        mqttClient.setKeepAlive(10);
        Serial.println("[MQTT] Connecting...");
        mqttClient.connect();
    }

    /* Flip the LED on/off */
    server.on("/flip", HTTP_POST, []() {
        DynamicJsonDocument data(1024);
        deserializeJson(data, server.arg(0));
        /* Control LED */
        if (data["power"]) {
            digitalWrite(SINGLE_LED, HIGH);
        } else {
            digitalWrite(SINGLE_LED, LOW);
        }
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.send(200, "text/html", "LED flipped.");
    });

    /* Return current LED state ("on" or "off") */
    server.on("/led", HTTP_GET, []() {
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.send(200, "text/plain", digitalRead(SINGLE_LED) ? "on" : "off"); 
    });

    /* Change RGB LED color */
    server.on("/rgb", HTTP_POST, []() {
        DynamicJsonDocument rgb(1024);
        deserializeJson(rgb, server.arg(0));
        /* Change LED color */
        rgbColor(rgb["r"], rgb["g"], rgb["b"]);
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.send(200, "text/html", "RGB changed.");
    });

    /* Read current voltage at LDR */
    server.on("/ldr", HTTP_GET, []() {
        float voltage = ldrVoltage();
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.send(200, "text/html", String(voltage));
    });

    /* Handle OPTIONS method and '404 Not Found' */
    server.onNotFound([]() {
        if (server.method() == HTTP_OPTIONS) {
            server.sendHeader("Access-Control-Allow-Origin", "*");
            server.sendHeader("Access-Control-Max-Age", "10000");
            server.sendHeader("Access-Control-Allow-Methods", "PUT,POST,GET,OPTIONS");
            server.sendHeader("Access-Control-Allow-Headers", "*");
            server.send(204);
        } else {
            server.send(404, "text/plain", "");
        }
    });

    /* Start the server */
    server.begin();
    Serial.println("[ESP8266] Web server started.");
    start = millis();
}

void loop() {
    /* Read LDR condition every 0.5 seconds */
    if (millis() - start >= 500) {
        float voltage = ldrVoltage();
        mqttClient.publish("/ldr", 0, true, String(voltage).c_str());
        start = millis();
    }
    server.handleClient();
}

/* MQTT connected callback */
void onMqttConnect(bool sessionPresent) {
    Serial.println("[MQTT] Client connected.");
    /* Subscribe to topics */
    mqttClient.subscribe("/flip", 0);
    mqttClient.subscribe("/rgb", 0);
    /* Reset the retained messages */
    mqttClient.publish("/led", 0, true);
    mqttClient.publish("/lwt", 0, true, NULL);
}

/* MQTT message callback */
void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t length, size_t index, size_t total) {
    /* Print out received payload */
    String topicString = topic;
    Serial.print("\n[MQTT] Topic: ");
    Serial.println(topic);
    Serial.print("[MQTT] Payload: ");
    String data = ((String)payload).substring(0, length);
    Serial.println(data);
    /* Perform action based on received topic */
    if (topicString == "/flip") {
        DynamicJsonDocument json(1024);
        deserializeJson(json, data);
        /* Control LED */
        if (json["power"]) {
            digitalWrite(SINGLE_LED, HIGH);
        } else if (!json["power"]) {
            digitalWrite(SINGLE_LED, LOW);
        }
        mqttClient.publish("/led", 0, true, digitalRead(SINGLE_LED) ? "on" : "off");
    } else if (topicString == "/rgb") {
        DynamicJsonDocument rgb(1024);
        deserializeJson(rgb, data);
        /* Change LED color */
        rgbColor(rgb["r"], rgb["g"], rgb["b"]);
    }
}

/* Change RB LED value */
void rgbColor(int red, int green, int blue) {
    analogWrite(RED, 1023 - red * 1023 / 255);
    analogWrite(GREEN, 1023 - green * 1023 / 255);
    analogWrite(BLUE, 1023 - blue * 1023 / 255);
}

/* Read voltage at LDR */
float ldrVoltage() {
    int sensorValue =  analogRead(A0);
    float voltage = sensorValue * (3.3 / 1023.0);
    return voltage;
}