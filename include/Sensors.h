#ifndef SENSORS_H
#define SENSORS_H
#include <DHT.h>

class SensorManager {
private:
    DHT dht;
    int pinCLK, pinDT, pinSW, pinFlame, pinJoyY;
    int lastCLK;
    int soilValue = 50; // Empezamos en 50%

public:
    SensorManager(int dhtP, int clk, int dt, int sw, int flame, int jY) 
        : dht(dhtP, DHT11), pinCLK(clk), pinDT(dt), pinSW(sw), pinFlame(flame), pinJoyY(jY) {}

    void begin() {
        dht.begin();
        pinMode(pinCLK, INPUT);
        pinMode(pinDT, INPUT);
        pinMode(pinSW, INPUT_PULLUP);
        pinMode(pinFlame, INPUT_PULLUP);
        pinMode(pinJoyY, INPUT);
        lastCLK = digitalRead(pinCLK);
    }

    float getT() { return dht.readTemperature(); }
    float getH() { return dht.readHumidity(); }
    
    // Lógica para el Encoder (Humedad Suelo)
    int getSoil() {
        int currentCLK = digitalRead(pinCLK);
        if (currentCLK != lastCLK && currentCLK == 1) {
            // Si el CLK cambió, revisamos DT para saber dirección
            if (digitalRead(pinDT) != currentCLK) {
                soilValue += 5; // Gira derecha: Sube humedad
            } else {
                soilValue -= 5; // Gira izquierda: Baja humedad
            }
            // Limitar entre 0 y 100
            if(soilValue > 100) soilValue = 100;
            if(soilValue < 0) soilValue = 0;
        }
        lastCLK = currentCLK;
        return soilValue;
    }

    bool isFire() { return digitalRead(pinFlame) == LOW; }

    int getMenuNav() {
        int val = analogRead(pinJoyY);
        if(val < 1000) return 0;
        if(val > 3000) return 1;
        return -1;
    }
};
#endif