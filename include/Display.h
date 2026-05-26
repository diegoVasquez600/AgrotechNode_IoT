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

// Paleta de Colores Industriales (Formato RGB565)
#define COLOR_BG      0x0825 // Fondo muy oscuro azul/gris
#define COLOR_PANEL   0x18E7 // Fondo de las cajas de datos
#define COLOR_HEADER  0x02F3 // Azul técnico profundo
#define COLOR_TEXT    0xFFFF // Blanco
#define COLOR_ACCENT  0x07E0 // Verde Lima (Estabilidad/OK)
#define COLOR_WARN    0xF800 // Rojo (Alerta)
#define COLOR_DATA    0x07FF // Cyan (Datos neutros)

class DisplayManager {
private:
    Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
    
    // Variables para caché (Evita redibujar y parpadear)
    float lastTemp = -999;
    float lastHum = -999;
    int lastSoil = -1;
    int lastRelay = -1;
    
    int lastWifi = -1;
    int lastMqtt = -1;
    String lastIp = "";
    int lastRssi = -999;

    void drawPanel(int x, int y, int w, int h, const char* title) {
        tft.drawRoundRect(x, y, w, h, 5, 0x5AEB); 
        tft.fillRoundRect(x+1, y+1, w-2, h-2, 5, COLOR_PANEL); 
        tft.fillRect(x+1, y+1, w-2, 22, COLOR_HEADER); 
        tft.drawFastHLine(x+1, y+23, w-2, 0x5AEB); 
        tft.setTextColor(COLOR_TEXT);
        tft.setTextSize(1);
        tft.setCursor(x + 8, y + 8);
        tft.print(title);
    }

public:
    DisplayManager() {}

    void begin() {
        tft.begin();
        tft.setRotation(3); // Orientación horizontal
        tft.fillScreen(COLOR_BG);
    }

    void showSplash() {
        tft.fillScreen(COLOR_BG);
        tft.drawRect(10, 10, 300, 220, COLOR_ACCENT);
        tft.setTextColor(COLOR_TEXT);
        tft.setTextSize(3);
        tft.setCursor(35, 90);
        tft.println("AgroTech_Node");
        tft.setTextSize(1);
        tft.setCursor(85, 140);
        tft.setTextColor(COLOR_DATA);
        tft.println("SISTEMA DE RESILIENCIA HIDRICA");
        delay(2500);
        
        tft.fillScreen(COLOR_BG);
        drawStaticUI();
    }

    void drawStaticUI() {
        tft.fillRect(0, 0, 320, 25, COLOR_HEADER);
        tft.setTextColor(COLOR_TEXT);
        tft.setTextSize(2);
        tft.setCursor(10, 5);
        tft.print("AGROTECH_NODE PoC");

        drawPanel(5, 32, 150, 95, "MICROCLIMA");
        drawPanel(160, 32, 155, 95, "SUSTRATO & CTRL");
        drawPanel(5, 132, 310, 100, "TELEMETRIA DE RED");
    }

    void updateDashboard(float temp, float hum, int soil, bool relay, bool wifiOk, bool mqttOk, String ip, int rssi) {
        tft.setTextSize(2);
        if (temp != lastTemp || isnan(lastTemp)) {
            tft.setTextColor(COLOR_DATA, COLOR_PANEL); 
            tft.setCursor(15, 65);
            tft.printf("%.1f C  ", temp);
            lastTemp = temp;
        }
        if (hum != lastHum || isnan(lastHum)) {
            tft.setTextColor(COLOR_DATA, COLOR_PANEL);
            tft.setCursor(15, 95);
            tft.printf("%.1f %%  ", hum);
            lastHum = hum;
        }

        if (soil != lastSoil) {
            tft.setTextColor(soil < 30 ? COLOR_WARN : COLOR_ACCENT, COLOR_PANEL);
            tft.setCursor(170, 65);
            tft.printf("H2O: %d%%  ", soil);
            lastSoil = soil;
        }
        if (relay != (lastRelay == 1)) {
            tft.setTextSize(1);
            tft.setCursor(170, 100);
            if (relay) {
                tft.setTextColor(COLOR_BG, COLOR_ACCENT); 
                tft.print(" BOMBA: REGANDO  ");
            } else {
                tft.setTextColor(COLOR_TEXT, COLOR_WARN); 
                tft.print(" BOMBA: REPOSO   ");
            }
            lastRelay = relay ? 1 : 0;
        }

        tft.setTextSize(2);
        if (wifiOk != (lastWifi == 1) || ip != lastIp || rssi != lastRssi) {
            tft.setTextColor(COLOR_TEXT, COLOR_PANEL);
            tft.setCursor(15, 165);
            tft.print("Wi-Fi: ");
            tft.setTextColor(wifiOk ? COLOR_ACCENT : COLOR_WARN, COLOR_PANEL);
            tft.print(wifiOk ? "ONLINE      " : "OFFLINE     ");

            tft.setTextColor(COLOR_TEXT, COLOR_PANEL);
            tft.setTextSize(1);
            tft.setCursor(15, 190);
            tft.printf("IP: %-15s", ip.c_str());

            tft.setCursor(15, 208);
            tft.printf("Senal RSSI: %d dBm  ", rssi);

            lastWifi = wifiOk ? 1 : 0;
            lastIp = ip;
            lastRssi = rssi;
        }

        if (mqttOk != (lastMqtt == 1)) {
            tft.setTextSize(2);
            tft.setTextColor(COLOR_TEXT, COLOR_PANEL);
            tft.setCursor(160, 165);
            tft.print("MQTT: ");
            tft.setTextColor(mqttOk ? COLOR_ACCENT : COLOR_WARN, COLOR_PANEL);
            tft.print(mqttOk ? "ONLINE " : "OFFLINE");
            lastMqtt = mqttOk ? 1 : 0;
        }
    }
};
#endif