/**
 * @file Display.h
 * @brief Interfaz HMI Local - Control TFT ST7789 (GMT020-02) y Boot Animation
 * @project AgroTech_Node PoC
 * @author Diego Alejandro Ríos Vásquez
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <math.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

// Pines SPI según esquemático oficial
#define TFT_CS 5
#define TFT_DC 2
#define TFT_RST 4
#define TFT_MOSI 23
#define TFT_SCLK 18

// Paleta SCADA Tech (RGB565)
#define COLOR_BG            0x0000 // Fondo Negro Puro para evitar parpadeo
#define COLOR_HEADER        0x0191 // Azul oscuro técnico
#define COLOR_PANEL_BG      0x0000 
#define COLOR_PANEL_BORDER  0x03EF // Azul técnico luminoso

#define COLOR_TEXT_BLUE     0x07FF // Cian nítido para etiquetas
#define COLOR_DATA_GREEN    0x07E0 // Verde menta para datos
#define COLOR_OK            0x07E0 // Verde (Estabilidad)
#define COLOR_WARN          0xF800 // Rojo (Alerta)

class DisplayManager {
private:
    Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
    
    float lastTemp = -999;
    float lastHum = -999;
    int lastSoil = -1;
    int lastRelay = -1;
    
    int lastWifi = -1;
    int lastMqtt = -1;
    String lastIp = "";
    int lastRssi = -999;

    void drawPanel(int x, int y, int w, int h, const char* title) {
        tft.drawRoundRect(x, y, w, h, 3, COLOR_PANEL_BORDER); 
        tft.fillRoundRect(x+1, y+1, w-2, 22, 3, COLOR_HEADER); // Barra de título
        tft.drawFastHLine(x+1, y+23, w-2, COLOR_PANEL_BORDER); 
        tft.setTextColor(0xFFFF); 
        tft.setTextSize(1);
        tft.setCursor(x + 8, y + 7);
        tft.print(title);
    }

public:
    DisplayManager() {}

    void begin() {
        tft.init(240, 320); // Resolución exacta del GMT020-02
        tft.setRotation(3); // 1 = Apaisado. Si te queda de cabeza físicamente, cámbialo a 3.
        tft.fillScreen(COLOR_BG);
        
        // NOTA: Si los colores se ven invertidos (ej. fondo blanco), descomenta la siguiente línea:
        // tft.invertDisplay(false); 
    }

    void showSplash() {
        tft.fillScreen(COLOR_BG);
        
        // Marco exterior
        tft.drawRect(5, 5, 310, 230, COLOR_PANEL_BORDER);
        tft.drawRect(7, 7, 306, 226, COLOR_PANEL_BORDER);
        
        tft.setTextColor(COLOR_TEXT_BLUE);
        tft.setTextSize(3);
        tft.setCursor(20, 50);
        tft.print("AgroTech_Node");
        
        tft.setTextSize(1);
        tft.setCursor(95, 90);
        tft.setTextColor(0xFFFF);
        tft.print("SISTEMA DE RESILIENCIA");

        // Barra de progreso animada
        tft.drawRect(40, 140, 240, 20, COLOR_TEXT_BLUE);
        tft.setCursor(40, 125);
        tft.setTextColor(COLOR_DATA_GREEN, COLOR_BG);
        tft.print("Iniciando Hardware...");
        
        for(int i = 0; i < 236; i += 20) {
            tft.fillRect(42, 142, i, 16, COLOR_DATA_GREEN);
            delay(120);
            if(i == 80) {
                tft.setCursor(40, 125);
                tft.print("Calibrando Sensores...  ");
            }
            if(i == 160) {
                tft.setCursor(40, 125);
                tft.print("Conectando a Red Edge...");
            }
        }
        delay(400);
        tft.fillScreen(COLOR_BG);
        drawStaticUI();
    }

    void drawStaticUI() {
        // Cabecera Principal
        tft.fillRect(0, 0, 320, 25, COLOR_HEADER);
        tft.setTextColor(0xFFFF);
        tft.setTextSize(2);
        tft.setCursor(10, 5);
        tft.print("AGROTECH_NODE PoC");

        // PANEL 1: MICROCLIMA
        drawPanel(5, 32, 150, 93, "MICROCLIMA");
        tft.setTextSize(1);
        tft.setCursor(15, 62); tft.setTextColor(COLOR_TEXT_BLUE, COLOR_PANEL_BG); tft.print("Temperatura");
        tft.setCursor(15, 92); tft.setTextColor(COLOR_TEXT_BLUE, COLOR_PANEL_BG); tft.print("Humedad/VPD");

        // PANEL 2: SUSTRATO
        drawPanel(160, 32, 155, 93, "SUSTRATO & CTRL");
        tft.setCursor(170, 62); tft.setTextColor(COLOR_TEXT_BLUE, COLOR_PANEL_BG); tft.print("Humedad Tierra");

        // PANEL 3: RED
        drawPanel(5, 130, 310, 105, "TELEMETRIA DE RED");
    }

    void updateDashboard(float temp, float hum, int soil, bool relay, bool wifiOk, bool mqttOk, String ip, int rssi) {
        tft.setTextSize(2);
        
        // Microclima
        if (temp != lastTemp || isnan(lastTemp)) {
            tft.setTextColor(COLOR_DATA_GREEN, COLOR_PANEL_BG); 
            tft.setCursor(15, 74);
            tft.printf("%.1f C  ", temp);
            lastTemp = temp;
        }
        if (hum != lastHum || isnan(lastHum)) {
            tft.setTextColor(COLOR_DATA_GREEN, COLOR_PANEL_BG);
            tft.setCursor(15, 104);
            tft.printf("%.1f %%  ", hum);
            lastHum = hum;
        }

        // Sustrato y Control
        if (soil != lastSoil) {
            tft.setTextColor(soil < 30 ? COLOR_WARN : COLOR_OK, COLOR_PANEL_BG);
            tft.setCursor(170, 74);
            tft.printf("%d%%     ", soil);
            lastSoil = soil;
        }
        if (relay != (lastRelay == 1)) {
            tft.setTextSize(2);
            tft.setCursor(170, 100);
            if (relay) {
                tft.setTextColor(COLOR_BG, COLOR_OK); 
                tft.print(" BOMBA ON  "); 
            } else {
                tft.setTextColor(COLOR_BG, COLOR_WARN); 
                tft.print(" BOMBA OFF "); 
            }
            lastRelay = relay ? 1 : 0;
        }

        // Red (Corregido para evitar solapamiento)
        tft.setTextSize(2);
        if (wifiOk != (lastWifi == 1) || ip != lastIp || rssi != lastRssi) {
            tft.setTextColor(COLOR_TEXT_BLUE, COLOR_PANEL_BG);
            tft.setCursor(15, 158);
            tft.print("Wi-Fi: ");
            tft.setTextColor(wifiOk ? COLOR_OK : COLOR_WARN, COLOR_PANEL_BG);
            
            // 1. ELIMINAMOS los espacios sobrantes que borraban el texto de la derecha
            tft.print(wifiOk ? "ONLINE " : "OFFLINE");

            tft.setTextColor(COLOR_TEXT_BLUE, COLOR_PANEL_BG);
            tft.setTextSize(1);
            tft.setCursor(15, 185);
            tft.printf("IP: %-15s", ip.c_str());

            tft.setCursor(15, 205);
            tft.printf("Senal RSSI: %d dBm  ", rssi);

            lastWifi = wifiOk ? 1 : 0;
            lastIp = ip;
            lastRssi = rssi;
        }

        // 2. MOVEMOS MQTT a la derecha (espacio libre) en dos lineas limpias
        if (mqttOk != (lastMqtt == 1)) {
            tft.setTextSize(2);
            tft.setTextColor(COLOR_TEXT_BLUE, COLOR_PANEL_BG);
            tft.setCursor(180, 175);
            tft.print("MQTT:");
            
            tft.setCursor(180, 195);
            tft.setTextColor(mqttOk ? COLOR_OK : COLOR_WARN, COLOR_PANEL_BG);
            tft.print(mqttOk ? "ONLINE " : "OFFLINE");
            lastMqtt = mqttOk ? 1 : 0;
        }
    }
};
#endif