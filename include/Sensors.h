/**
 * @file Sensors.h
 * @brief Gestión de Percepción y Edge Computing (Filtro Promedio Móvil k=5)
 * @project AgroTech_Node: Sistema de Resiliencia Hídrica y Monitoreo Térmico (Fase PoC y MVP)
 * @author Diego Alejandro Ríos Vásquez
 * @instructor Mg. Bernardo Molina Zuluaga
 * @course Optativa I: Internet de las Cosas
 * @institution Institución Universitaria Pascual Bravo
 * @date Mayo de 2026
 */

#ifndef SENSORS_H
#define SENSORS_H

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

class SensorManager {
private:
    Adafruit_BME280 bme;
    int pinSoilADC;
    
    // Valores de calibración del sensor capacitivo (Ajustar en campo)
    const int VALOR_SECO = 3500;  // Lectura ADC cuando la tierra está seca
    const int VALOR_AGUA = 1500;  // Lectura ADC cuando está sumergido

public:
    SensorManager(int soilPin) : pinSoilADC(soilPin) {}

    void begin() {
        // Inicializar I2C en los pines definidos en el esquemático (SDA=21, SCL=22)
        Wire.begin(21, 22);
        
        if (!bme.begin(0x76, &Wire)) {
            Serial.println("Error: No se encuentra el sensor BME280!");
        }
        
        pinMode(pinSoilADC, INPUT);
    }

    float getT() { 
        return bme.readTemperature(); 
    }
    
    float getH() { 
        return bme.readHumidity(); 
    }

    // Edge Computing: Implementación del Filtro de Promedio Móvil (k=5)
    int getSoil() {
        int lecturas[5];      // Ventana de muestreo k=5
        int total = 0;
        
        for(int i=0; i<5; i++) {
            lecturas[i] = analogRead(pinSoilADC); // Captura del dato crudo
            total += lecturas[i];
            delay(10);          // Estabilización entre muestras
        }
        
        int promedio = total / 5; // Valor filtrado
        
        // Mapear la lectura analógica filtrada a un porcentaje (0% - 100%)
        int porcentaje = map(promedio, VALOR_SECO, VALOR_AGUA, 0, 100);
        
        // Restringir los valores entre 0 y 100 por si hay picos extremos
        if(porcentaje > 100) porcentaje = 100;
        if(porcentaje < 0) porcentaje = 0;
        
        return porcentaje;
    }
};
#endif