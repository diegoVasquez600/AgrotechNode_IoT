/**
 * @file main.cpp
 * @brief Firmware Edge Computing - Núcleo de Control y Telemetría
 * @project AgroTech_Node PoC
 * @author Diego Alejandro Ríos Vásquez
 */

#include <Arduino.h>
#include "Sensors.h"
#include "Display.h"
#include "NetworkManager.h"

// Datos de Conexión (Ajusta el Broker a la IP de tu Raspberry Pi Gateway)
const char* WIFI_SSID = "AgroTech_IoT";
const char* WIFI_PASS = "Wifi_Pass";
const char* MQTT_BROKER_IP = "192.168.1.113";
const char* MQTT_USER = "user";
const char* MQTT_PASS = "pass";

const int PIN_SOIL_ADC = 34; // Sensor Capacitivo v1.2
const int PIN_RELAY = 27;    // Actuador Bomba DC
const int UMBRAL_MARCHITEZ = 30; // 30% enciende la bomba

SensorManager sensors(PIN_SOIL_ADC);
DisplayManager ui;
NetworkManager net(WIFI_SSID, WIFI_PASS, MQTT_BROKER_IP, MQTT_USER, MQTT_PASS);

unsigned long tUpdate = 0;

void setup() {
    Serial.begin(115200);
    
    pinMode(PIN_RELAY, OUTPUT);
    digitalWrite(PIN_RELAY, LOW); // Fail-Safe
    
    sensors.begin();
    ui.begin();
    net.begin();
    
    ui.showSplash();
}

void loop() {
    net.loop();

    float temp = sensors.getT();
    float hum = sensors.getH();
    int soil_percent = sensors.getSoil();
    
    bool valveStatus = (soil_percent < UMBRAL_MARCHITEZ);
    digitalWrite(PIN_RELAY, valveStatus ? HIGH : LOW);

    if(millis() - tUpdate > 1000) {
        ui.updateDashboard(temp, hum, soil_percent, valveStatus, net.isWifiOk(), net.isMqttOk(), net.getWifiIp(), net.getWifiRssi());
        
        static int sendCount = 0;
        if(++sendCount >= 5) { 
            net.sendJson(temp, hum, soil_percent, valveStatus, net.getWifiIp(), net.getWifiSsid(), net.getWifiRssi(), net.getWifiQualityText());
            sendCount = 0;
        }
        tUpdate = millis();
    }
}
