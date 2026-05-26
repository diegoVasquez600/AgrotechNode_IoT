/**
 * @file Display.h
 * @brief Interfaz HMI Local - Control de Pantalla TFT y Renderizado de Estados
 * @project AgroTech_Node: Sistema de Resiliencia Hídrica y Monitoreo Térmico (Fase PoC y MVP)
 * @author Diego Alejandro Ríos Vásquez
 * @instructor Mg. Bernardo Molina Zuluaga
 * @course Optativa I: Internet de las Cosas
 * @institution Institución Universitaria Pascual Bravo
 * @date Mayo de 2026
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <math.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

// Pines SPI según esquemático oficial
#define TFT_CS 5
#define TFT_DC 2
#define TFT_RST 4
#define TFT_MOSI 23
#define TFT_SCLK 18

class DisplayManager {
private:
    Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
    bool lastWifiOk = false;
    bool lastMqttOk = false;
    bool headerStatusInit = false;
    bool dashboardCacheInit = false;
    bool networkCacheInit = false;

    float lastTemp = NAN;
    float lastHum = NAN;
    int lastSoil = -1;
    bool lastRelay = false;

    String lastSsid = "";
    String lastIp = "";
    int lastRssi = -999;
    String lastQuality = "";
    String lastMqttHost = "";
    String lastMqttState = "";
    bool lastMqttStateOk = false;

    void invalidateDynamicCache() {
        dashboardCacheInit = false;
        networkCacheInit = false;
        lastTemp = NAN;
        lastHum = NAN;
        lastSoil = -1;
        lastRelay = false;
    }

    void drawWifiIcon(int x, int y, bool ok) {
        uint16_t color = ok ? ILI9341_GREEN : ILI9341_RED;
        tft.drawCircle(x, y, 3, color);
        tft.drawCircle(x, y, 7, color);
        tft.fillCircle(x, y, 2, color);
    }

    void drawHeader(const char* title, bool wifiOk, bool mqttOk) {
        tft.fillRect(0, 0, 320, 40, ILI9341_BLUE);
        tft.setTextColor(ILI9341_WHITE, ILI9341_BLUE);
        tft.setTextSize(2);
        tft.setCursor(10, 12);
        tft.print(title);
        drawWifiIcon(245, 19, wifiOk);
        headerStatusInit = true;
    }

    void drawFooterHint(const char* text) {
        tft.fillRect(0, 218, 320, 22, ILI9341_BLACK);
        tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
        tft.setTextSize(1);
        tft.setCursor(8, 225);
        tft.print(text);
    }

public:
    DisplayManager() {}

    void begin() {
        tft.begin();
        tft.setRotation(1); // Apaisado
        tft.fillScreen(ILI9341_BLACK);
        invalidateDynamicCache();
    }

    void showSplash() {
        tft.fillScreen(ILI9341_BLACK);
        tft.drawRect(5, 5, 310, 230, ILI9341_GREEN);
        tft.setTextColor(ILI9341_WHITE);
        tft.setTextSize(3);
        tft.setCursor(20, 80);
        tft.println("AgroTech Node PoC");
        tft.setTextSize(2);
        tft.setCursor(80, 140);
        tft.println("INITIALIZING...");
        delay(3000);
    }

    void drawDashboardFrame() {
        tft.fillScreen(ILI9341_BLACK);
        drawHeader("AGROTECH NODO 1", lastWifiOk, lastMqttOk);
        tft.drawRoundRect(8, 48, 304, 164, 8, ILI9341_WHITE);
        drawFooterHint("MODO: MONITOREO CONTINUO");
        dashboardCacheInit = false;
    }

    void drawNetworkFrame() {
        tft.fillScreen(ILI9341_BLACK);
        drawHeader("ESTADO DE RED", lastWifiOk, lastMqttOk);
        tft.drawRoundRect(8, 48, 304, 164, 8, ILI9341_CYAN);
        drawFooterHint("MODO: DIAGNOSTICO DE ENLACE");
        networkCacheInit = false;
    }

    void updateData(float temp, float hum, int soil, bool relay) {
        tft.setTextSize(2);
        if (!dashboardCacheInit || fabsf(temp - lastTemp) >= 0.2f) {
            tft.fillRect(16, 78, 286, 24, ILI9341_BLACK);
            tft.setCursor(16, 78);
            tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
            tft.printf("Temp:  %.1f C", temp);
            lastTemp = temp;
        }
        if (!dashboardCacheInit || fabsf(hum - lastHum) >= 0.2f) {
            tft.fillRect(16, 108, 286, 24, ILI9341_BLACK);
            tft.setCursor(16, 108);
            tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
            tft.printf("VPD/Hum: %.1f %%", hum);
            lastHum = hum;
        }
        if (!dashboardCacheInit || soil != lastSoil) {
            tft.fillRect(16, 138, 286, 24, ILI9341_BLACK);
            tft.setCursor(16, 138);
            tft.setTextColor(ILI9341_MAGENTA, ILI9341_BLACK);
            tft.printf("Suelo: %d %%", soil);
            lastSoil = soil;
        }
        if (!dashboardCacheInit || relay != lastRelay) {
            tft.fillRect(16, 172, 286, 24, ILI9341_BLACK);
            tft.setCursor(16, 172);
            if (relay) {
                tft.setTextColor(ILI9341_BLACK, ILI9341_GREEN);
                tft.println(" BOMBA: REGANDO ");
            } else {
                tft.setTextColor(ILI9341_WHITE, ILI9341_RED);
                tft.println(" BOMBA: REPOSO  ");
            }
            lastRelay = relay;
        }
        dashboardCacheInit = true;
    }

    void updateNetworkStatus(bool wifiOk, bool mqttOk, String ip, int rssi) {
        tft.setTextSize(2);
        if (!networkCacheInit) {
            tft.setCursor(16, 76); tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK); tft.print("IP: "); 
            tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK); tft.println(ip);
            
            tft.setCursor(16, 112); tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK); tft.print("RSSI: ");
            tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK); tft.printf("%d dBm", rssi);
            
            tft.setCursor(16, 150); tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK); tft.print("MQTT: ");
            tft.setTextColor(mqttOk ? ILI9341_GREEN : ILI9341_RED, ILI9341_BLACK); 
            tft.println(mqttOk ? "CONECTADO" : "DESCONECTADO");
        }
        networkCacheInit = true;
    }
};
#endif