#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <WiFi.h>
#include <PubSubClient.h>

class NetworkManager {
private:
    WiFiClient espClient;
    PubSubClient client;
    const char* ssid;
    const char* password;
    const char* mqtt_server;
    const char* mqtt_user;
    const char* mqtt_pass;
    unsigned long lastWifiAttempt = 0;
    unsigned long lastMqttAttempt = 0;

    void connectWifi(bool force = false) {
        const unsigned long retryMs = 10000;
        if (!force && (millis() - lastWifiAttempt) < retryMs) {
            return;
        }

        lastWifiAttempt = millis();
        Serial.print("[WiFi] Conectando a: ");
        Serial.println(ssid);

        WiFi.disconnect(true, true);
        delay(100);
        WiFi.mode(WIFI_STA);
        WiFi.setSleep(false);
        WiFi.begin(ssid, password);
    }

    const char* wifiStatusText(wl_status_t st) {
        switch (st) {
            case WL_IDLE_STATUS: return "IDLE";
            case WL_NO_SSID_AVAIL: return "SSID no disponible";
            case WL_SCAN_COMPLETED: return "SCAN completo";
            case WL_CONNECTED: return "Conectado";
            case WL_CONNECT_FAILED: return "Fallo auth/clave";
            case WL_CONNECTION_LOST: return "Conexion perdida";
            case WL_DISCONNECTED: return "Desconectado";
            default: return "Estado desconocido";
        }
    }

public:
    NetworkManager(const char* _ssid, const char* _pass, const char* _server, const char* _mqttUser = nullptr, const char* _mqttPass = nullptr)
        : client(espClient), ssid(_ssid), password(_pass), mqtt_server(_server), mqtt_user(_mqttUser), mqtt_pass(_mqttPass) {}

    void begin() {
        WiFi.mode(WIFI_STA);
        WiFi.setSleep(false);
        connectWifi(true);
        // El puerto estándar es 1883
        client.setServer(mqtt_server, 1883);
    }

    void reconnect() {
        // Solo intenta reconectar si hay WiFi pero no hay MQTT
        if (!client.connected() && WiFi.status() == WL_CONNECTED) {
            const unsigned long retryMs = 5000;
            if (millis() - lastMqttAttempt < retryMs) {
                return;
            }
            lastMqttAttempt = millis();

            Serial.print("Intentando conexión MQTT...");
            Serial.print(" broker=");
            Serial.print(mqtt_server);
            Serial.print(":");
            Serial.println(1883);

            String clientId = "ESP32-Agrotech-";
            clientId += String(random(0xffff), HEX);
            
            bool connected = false;
            if (mqtt_user != nullptr && mqtt_pass != nullptr) {
                connected = client.connect(clientId.c_str(), mqtt_user, mqtt_pass);
            } else {
                connected = client.connect(clientId.c_str());
            }

            if (connected) {
                Serial.println("¡Conectado!");
            } else {
                Serial.print("Fallo, rc=");
                Serial.println(client.state());
                Serial.println("Verifica IP del broker, puerto 1883 y si Mosquitto permite conexion anonima o requiere usuario/clave.");
            }
        }
    }

    void loop() {
        wl_status_t st = WiFi.status();
        if (st != WL_CONNECTED) {
            static unsigned long lastStatusLog = 0;
            if (millis() - lastStatusLog > 4000) {
                Serial.print("[WiFi] Estado: ");
                Serial.println(wifiStatusText(st));
                lastStatusLog = millis();
            }
            connectWifi();
            return;
        }

        if (!client.connected()) reconnect();
        client.loop();
    }

    // --- Getters para la interfaz de usuario ---
    bool isWifiOk() { return WiFi.status() == WL_CONNECTED; }
    bool isMqttOk() { return client.connected(); }

    String getWifiSsid() { return String(ssid); }
    
    String getWifiIp() {
        return (WiFi.status() == WL_CONNECTED) ? WiFi.localIP().toString() : "0.0.0.0";
    }

    int getWifiRssi() {
        return (WiFi.status() == WL_CONNECTED) ? WiFi.RSSI() : -127;
    }

    String getWifiQualityText() {
        int rssi = getWifiRssi();
        if (rssi >= -55) return "Excelente";
        if (rssi >= -67) return "Buena";
        if (rssi >= -75) return "Media";
        if (rssi >= -85) return "Debil";
        return "Sin Senal";
    }

    // Cambiado a const char* para que funcione directo con tft.printf
    const char* getMqttHost() { return mqtt_server; }
    int getMqttPort() { return 1883; }

    String getMqttStateText() {
        if (WiFi.status() != WL_CONNECTED) return "Sin WiFi";
        switch (client.state()) {
            case MQTT_CONNECTION_TIMEOUT:      return "Timeout";
            case MQTT_CONNECTION_LOST:         return "Perdida";
            case MQTT_CONNECT_FAILED:          return "Fallo Red";
            case MQTT_CONNECT_BAD_CLIENT_ID:   return "ClientID";
            case MQTT_CONNECT_BAD_CREDENTIALS: return "Credenciales";
            case MQTT_CONNECT_UNAUTHORIZED:    return "No autorizado";
            case MQTT_CONNECT_UNAVAILABLE:     return "Broker off";
            case MQTT_DISCONNECTED:            return "Desconectado";
            case MQTT_CONNECTED:               return "Conectado";
            case MQTT_CONNECT_BAD_PROTOCOL:    return "Protocolo";
            default:                           return "Error";
        }
    }

    void sendJson(float t, float h, int s, bool r, const String& ip, const String& wifiSsid, int rssi, const String& qual) {
        if (client.connected()) {
            // Formato alineado con el dashboard de Node-RED.
            String msg = "{\"t\":" + String(t,1)
                       + ",\"h\":" + String(h,1)
                       + ",\"s\":" + String(s)
                       + ",\"r\":" + String(r ? "true" : "false")
                       + ",\"ip\":\"" + ip + "\""
                       + ",\"ssid\":\"" + wifiSsid + "\""
                       + ",\"rssi\":" + String(rssi)
                       + ",\"qual\":\"" + qual + "\"}";
            client.publish("pascual/agrotech", msg.c_str());
        }
    }
};
#endif