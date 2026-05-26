#include <Arduino.h>
#include "Sensors.h"
#include "Display.h"
#include "NetworkManager.h"

// Datos de Conexión
const char* WIFI_SSID = "AgroTech_IoT";
const char* WIFI_PASS = "HAILKn0x64_Dv*";
const char* MQTT_BROKER_IP = "192.168.1.113";
const char* MQTT_USER = "mrknox";
const char* MQTT_PASS = "HAILKn0x64";

// SensorManager(DHT, CLK, DT, SW_ENCODER, FLAME, JOY_Y)
SensorManager sensors(26, 34, 35, 33, 27, 32);
DisplayManager ui;
NetworkManager net(WIFI_SSID, WIFI_PASS, MQTT_BROKER_IP, MQTT_USER, MQTT_PASS);

unsigned long tUpdate = 0;
enum UiScreen { UI_DASHBOARD, UI_NETWORK };
UiScreen currentScreen = UI_DASHBOARD;

int lastNavState = -1;
unsigned long tLastNav = 0;
bool navArmed = false;

void setup() {
    Serial.begin(115200);
    sensors.begin();
    ui.begin();
    net.begin();
    pinMode(25, OUTPUT); // Relé
    
    ui.showSplash();
    ui.drawDashboardFrame();
}

void loop() {
    net.loop();

    // Lógica de Navegación con Joystick
    int nav = sensors.getMenuNav();
    if (!navArmed) {
        if (nav == -1) navArmed = true;
    } else {
        if (nav != -1 && lastNavState == -1 && (millis() - tLastNav) > 400) {
            currentScreen = (currentScreen == UI_DASHBOARD) ? UI_NETWORK : UI_DASHBOARD;
            if (currentScreen == UI_DASHBOARD) ui.drawDashboardFrame();
            else ui.drawNetworkFrame();
            tLastNav = millis();
        }
    }
    lastNavState = nav;

    // Seguridad: Fuego
    if(sensors.isFire()) {
        ui.showFireAlert();
        digitalWrite(25, LOW);
        while(sensors.isFire());
        ui.begin(); 
        if (currentScreen == UI_DASHBOARD) ui.drawDashboardFrame();
        else ui.drawNetworkFrame();
    }

    // Control de Riego
    float temp = sensors.getT();
    float hum = sensors.getH();
    int soil = sensors.getSoil();
    bool valve = (soil < 30);
    digitalWrite(25, valve ? HIGH : LOW);

    String wifiIp = net.getWifiIp();
    String wifiSsid = net.getWifiSsid();
    int wifiRssi = net.getWifiRssi();
    String wifiQual = net.getWifiQualityText();

    // Actualización de Pantalla y Envío IoT
    if(millis() - tUpdate > 900) {
        String wStat = net.isWifiOk() ? "W:OK" : "W:ERR";
        String mStat = net.isMqttOk() ? "M:OK" : "M:ERR";

        if (currentScreen == UI_DASHBOARD) {
            ui.updateData(temp, hum, soil, valve, wStat, mStat);
        } else {
            ui.updateNetworkStatus(
                net.isWifiOk(), net.isMqttOk(), wifiSsid,
                wifiIp, wifiRssi, wifiQual,
                net.getMqttHost(), net.getMqttPort(), net.getMqttStateText()
            );
        }
        
        static int sendCount = 0;
        if(++sendCount >= 6) { // Cada ~5.4 segundos
            net.sendJson(temp, hum, soil, valve, wifiIp, wifiSsid, wifiRssi, wifiQual);
            sendCount = 0;
        }
        tUpdate = millis();
    }
}