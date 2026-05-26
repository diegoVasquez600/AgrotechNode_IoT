/**
 * @file main.cpp
 * @brief Firmware Edge Computing - Núcleo de Control y Telemetría
 * @project AgroTech_Node: Sistema de Resiliencia Hídrica y Monitoreo Térmico (Fase PoC y MVP)
 * @author Diego Alejandro Ríos Vásquez
 * @instructor Mg. Bernardo Molina Zuluaga
 * @course Optativa I: Internet de las Cosas
 * @institution Institución Universitaria Pascual Bravo
 * @date Mayo de 2026
 */

#include <Arduino.h>
#include "Sensors.h"
#include "Display.h"
#include "NetworkManager.h"

// Datos de Conexión (Ajusta el Broker a la IP de tu Raspberry Pi Gateway)
const char* WIFI_SSID = "AgroTech_IoT";
const char* WIFI_PASS = "HAILKn0x64_Dv*";
const char* MQTT_BROKER_IP = "192.168.1.113";
const char* MQTT_USER = "mrknox";
const char* MQTT_PASS = "HAILKn0x64";

// Definición de Pines según el Esquemático Industrial
const int PIN_SOIL_ADC = 34; // Sensor Capacitivo v1.2 (Señal Analógica)
const int PIN_RELAY = 27;    // Actuador Bomba DC (Relé JQC-3F)
const int UMBRAL_MARCHITEZ = 30; // Si baja del 30%, enciende la bomba

SensorManager sensors(PIN_SOIL_ADC);
DisplayManager ui;
NetworkManager net(WIFI_SSID, WIFI_PASS, MQTT_BROKER_IP, MQTT_USER, MQTT_PASS);

unsigned long tUpdate = 0;

void setup() {
    Serial.begin(115200);
    
    // Configuración Lógica de Actuación (Fail-Safe)
    pinMode(PIN_RELAY, OUTPUT);
    digitalWrite(PIN_RELAY, LOW); // Normalmente Abierto (NO) apagado por defecto
    
    sensors.begin();
    ui.begin();
    net.begin();
    
    ui.showSplash();
}

void loop() {
    net.loop();

    // 1. Edge Computing: Adquisición de Smart Data
    float temp = sensors.getT();
    float hum = sensors.getH();
    int soil_percent = sensors.getSoil(); // Ya viene con Filtro de Promedio Móvil
    
    // 2. Lógica de Control (Histéresis básica y actuación)
    bool valveStatus = (soil_percent < UMBRAL_MARCHITEZ);
    digitalWrite(PIN_RELAY, valveStatus ? HIGH : LOW);

    // 3. Actualización de Interfaz HMI Unificada (Dashboard Industrial Sin Parpadeo)
    if(millis() - tUpdate > 1000) {
        ui.updateDashboard(temp, hum, soil_percent, valveStatus, net.isWifiOk(), net.isMqttOk(), net.getWifiIp(), net.getWifiRssi());
        
        static int sendCount = 0;
        if(++sendCount >= 5) { // Publicar en MQTT cada 5 segundos
            net.sendJson(temp, hum, soil_percent, valveStatus, net.getWifiIp(), net.getWifiSsid(), net.getWifiRssi(), net.getWifiQualityText());
            sendCount = 0;
        }
        tUpdate = millis();
    }
}