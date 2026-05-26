#ifndef DISPLAY_H
#define DISPLAY_H

#include <math.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

#define TFT_CS 5
#define TFT_DC 2
#define TFT_RST 4
#define TFT_MOSI 23
#define TFT_SCLK 18

class DisplayManager {
private:
    Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
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
    int lastMqttPort = -1;
    String lastMqttState = "";
    bool lastMqttStateOk = false;

    void invalidateDynamicCache() {
        dashboardCacheInit = false;
        networkCacheInit = false;
        lastTemp = NAN;
        lastHum = NAN;
        lastSoil = -1;
        lastRelay = false;
        lastSsid = "";
        lastIp = "";
        lastRssi = -999;
        lastQuality = "";
        lastMqttHost = "";
        lastMqttPort = -1;
        lastMqttState = "";
        lastMqttStateOk = false;
    }

    void drawWifiIcon(int x, int y, bool ok) {
        uint16_t color = ok ? ST77XX_GREEN : ST77XX_RED;
        tft.drawCircle(x, y, 3, color);
        tft.drawCircle(x, y, 7, color);
        tft.drawCircle(x, y, 11, color);
        tft.fillCircle(x, y, 2, color);
    }

    void drawMqttIcon(int x, int y, bool ok) {
        uint16_t color = ok ? ST77XX_GREEN : ST77XX_RED;
        tft.drawRoundRect(x - 8, y - 8, 16, 16, 3, color);
        tft.fillRect(x - 2, y - 2, 4, 4, color);
        tft.drawFastHLine(x + 8, y, 7, color);
    }

    void drawHeader(const char* title, bool wifiOk, bool mqttOk) {
        tft.fillRect(0, 0, 320, 40, ST77XX_BLUE);
        tft.drawFastHLine(0, 40, 320, ST77XX_WHITE);

        tft.setTextColor(ST77XX_WHITE, ST77XX_BLUE);
        tft.setTextSize(2);
        tft.setCursor(10, 12);
        tft.print(title);

        drawWifiIcon(245, 19, wifiOk);
        tft.setTextSize(1);
        tft.setCursor(258, 14);
        tft.print("WIFI");

        drawMqttIcon(292, 19, mqttOk);
        tft.setCursor(304, 14);
        tft.print("MQ");

        lastWifiOk = wifiOk;
        lastMqttOk = mqttOk;
        headerStatusInit = true;
    }

    void updateHeaderStatus(bool wifiOk, bool mqttOk) {
        if (headerStatusInit && wifiOk == lastWifiOk && mqttOk == lastMqttOk) {
            return;
        }

        tft.fillRect(232, 5, 86, 28, ST77XX_BLUE);
        drawWifiIcon(245, 19, wifiOk);
        tft.setTextSize(1);
        tft.setTextColor(ST77XX_WHITE, ST77XX_BLUE);
        tft.setCursor(258, 14);
        tft.print("WIFI");

        drawMqttIcon(292, 19, mqttOk);
        tft.setCursor(304, 14);
        tft.print("MQ");

        lastWifiOk = wifiOk;
        lastMqttOk = mqttOk;
        headerStatusInit = true;
    }

    void drawFooterHint(const char* text) {
        tft.fillRect(0, 218, 320, 22, ST77XX_BLACK);
        tft.drawFastHLine(0, 218, 320, ST77XX_WHITE);
        tft.setTextSize(1);
        tft.setTextColor(ST77XX_YELLOW, ST77XX_BLACK);
        tft.setCursor(8, 225);
        tft.print(text);
    }

public:
    DisplayManager() {}

    void begin() {
        tft.init(240, 320);
        tft.setRotation(1);
        tft.fillScreen(ST77XX_BLACK);
        invalidateDynamicCache();
    }

    void showSplash() {
        tft.fillScreen(ST77XX_BLACK);
        tft.drawRect(5, 5, 310, 230, ST77XX_GREEN);
        tft.setTextColor(ST77XX_WHITE);
        tft.setTextSize(3);
        tft.setCursor(60, 80);
        tft.println("AGROTECH NODE");
        tft.setTextSize(2);
        tft.setCursor(80, 120);
        tft.println("INICIALIZANDO...");
        delay(3000);
        tft.fillScreen(ST77XX_BLACK);
    }

    void drawStaticUI() {
        drawDashboardFrame();
    }

    void drawDashboardFrame() {
        tft.fillScreen(ST77XX_BLACK);
        drawHeader("AGROTECH IoT", false, false);

        tft.drawRoundRect(8, 48, 304, 164, 8, ST77XX_WHITE);
        tft.drawFastHLine(8, 132, 304, ST77XX_GREEN);

        drawFooterHint("JOYSTICK: SUBE/BAJA PARA VER RED");
        dashboardCacheInit = false;
    }

    void drawNetworkFrame() {
        tft.fillScreen(ST77XX_BLACK);
        drawHeader("ESTADO RED", false, false);

        tft.drawRoundRect(8, 48, 304, 164, 8, ST77XX_CYAN);
        tft.fillRect(9, 49, 302, 18, ST77XX_BLUE);
        tft.setTextColor(ST77XX_WHITE, ST77XX_BLUE);
        tft.setTextSize(1);
        tft.setCursor(16, 54);
        tft.print("DETALLE WiFi / MQTT");

        drawFooterHint("JOYSTICK: SUBE/BAJA PARA VOLVER");
        networkCacheInit = false;
    }

    void updateData(float temp, float hum, int soil, bool relay, String wifiStr, String mqttStr) {
        bool wifiOk = wifiStr.indexOf("OK") >= 0;
        bool mqttOk = mqttStr.indexOf("OK") >= 0;
        updateHeaderStatus(wifiOk, mqttOk);

        tft.setTextSize(2);
        bool tempChanged = !dashboardCacheInit || isnan(lastTemp) || fabsf(temp - lastTemp) >= 0.2f;
        bool humChanged = !dashboardCacheInit || isnan(lastHum) || fabsf(hum - lastHum) >= 0.2f;
        bool soilChanged = !dashboardCacheInit || soil != lastSoil;
        bool relayChanged = !dashboardCacheInit || relay != lastRelay;

        if (tempChanged) {
            tft.fillRect(16, 78, 286, 24, ST77XX_BLACK);
            tft.setCursor(16, 78);
            tft.setTextColor(ST77XX_CYAN, ST77XX_BLACK);
            tft.printf("Temp:  %.1f C ", temp);
            lastTemp = temp;
        }

        if (humChanged) {
            tft.fillRect(16, 108, 286, 24, ST77XX_BLACK);
            tft.setCursor(16, 108);
            tft.setTextColor(ST77XX_CYAN, ST77XX_BLACK);
            tft.printf("Aire:  %.1f %% ", hum);
            lastHum = hum;
        }

        if (soilChanged) {
            tft.fillRect(16, 138, 286, 24, ST77XX_BLACK);
            tft.setCursor(16, 138);
            tft.setTextColor(ST77XX_MAGENTA, ST77XX_BLACK);
            tft.printf("Suelo: %d %%  ", soil);
            lastSoil = soil;
        }

        if (relayChanged) {
            tft.fillRect(16, 172, 286, 24, ST77XX_BLACK);
            tft.setCursor(16, 172);
            if (relay) {
                tft.setTextColor(ST77XX_BLACK, ST77XX_GREEN);
                tft.println(" BOMBA: ACTIVADA ");
            } else {
                tft.setTextColor(ST77XX_WHITE, ST77XX_RED);
                tft.println(" BOMBA: APAGADA  ");
            }
            lastRelay = relay;
        }

        dashboardCacheInit = true;
    }

    void updateNetworkStatus(
        bool wifiOk,
        bool mqttOk,
        const String& ssid,
        const String& ip,
        int rssi,
        const String& quality,
        const String& mqttHost,
        int mqttPort,
        const String& mqttState
    ) {
        updateHeaderStatus(wifiOk, mqttOk);

        tft.setTextSize(1);

        bool ssidChanged = !networkCacheInit || ssid != lastSsid;
        bool ipChanged = !networkCacheInit || ip != lastIp;
        bool rssiChanged = !networkCacheInit || (abs(rssi - lastRssi) >= 2) || quality != lastQuality;
        bool brokerChanged = !networkCacheInit || mqttHost != lastMqttHost || mqttPort != lastMqttPort;
        bool mqttStateChanged = !networkCacheInit || mqttState != lastMqttState || mqttOk != lastMqttStateOk;

        if (ssidChanged) {
            tft.fillRect(16, 76, 286, 12, ST77XX_BLACK);
            tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
            tft.setCursor(16, 76);
            tft.print("SSID: ");
            tft.setTextColor(ST77XX_CYAN, ST77XX_BLACK);
            tft.print(ssid);
            lastSsid = ssid;
        }

        if (ipChanged) {
            tft.fillRect(16, 94, 286, 12, ST77XX_BLACK);
            tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
            tft.setCursor(16, 94);
            tft.print("IP:   ");
            tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
            tft.print(ip);
            lastIp = ip;
        }

        if (rssiChanged) {
            tft.fillRect(16, 112, 286, 12, ST77XX_BLACK);
            tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
            tft.setCursor(16, 112);
            tft.print("RSSI: ");
            tft.setTextColor(ST77XX_YELLOW, ST77XX_BLACK);
            tft.printf("%d dBm (%s)", rssi, quality.c_str());
            lastRssi = rssi;
            lastQuality = quality;
        }

        if (brokerChanged) {
            tft.fillRect(16, 136, 286, 26, ST77XX_BLACK);
            tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
            tft.setCursor(16, 136);
            tft.print("Broker:");
            tft.setTextColor(ST77XX_CYAN, ST77XX_BLACK);
            tft.setCursor(16, 152);
            tft.print(mqttHost);
            tft.print(":");
            tft.print(mqttPort);
            lastMqttHost = mqttHost;
            lastMqttPort = mqttPort;
        }

        if (mqttStateChanged) {
            tft.fillRect(16, 178, 286, 12, ST77XX_BLACK);
            tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
            tft.setCursor(16, 178);
            tft.print("MQTT Estado:");
            tft.setTextColor(mqttOk ? ST77XX_GREEN : ST77XX_RED, ST77XX_BLACK);
            tft.setCursor(104, 178);
            tft.print(mqttState);
            lastMqttState = mqttState;
            lastMqttStateOk = mqttOk;
        }

        networkCacheInit = true;
    }

    void showFireAlert() {
        tft.fillScreen(ST77XX_RED);
        tft.setTextColor(ST77XX_WHITE, ST77XX_RED);
        tft.setTextSize(4);
        tft.setCursor(40, 100);
        tft.println("FUEGO!!!");
    }
};

#endif