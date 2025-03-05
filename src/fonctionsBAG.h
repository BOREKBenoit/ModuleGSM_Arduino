#ifndef FONCTIONSBAG_H      // Vérifie si CRC_H n'a pas encore été défini
#define FONCTIONSBAG_H      // Définition de CRC_H pour éviter l'inclusion multiple

#include <Arduino.h>
#include <Adafruit_BMP280.h>
#include <Wire.h>

extern Adafruit_BMP280 bmp;

void decode();
void C3Init(uint8_t Addr);
void bmpInit(uint8_t BMP_I2C_Addr);
float bmpReadPressure();
void C3SendConfig(uint8_t Addr);
void C3Read(uint8_t Addr, float &Temp, float &Hum);

#endif  // Fin du bloc de garde d'inclusion
