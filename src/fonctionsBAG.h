#ifndef FONCTIONSBAG_H      // Vérifie si CRC_H n'a pas encore été défini
#define FONCTIONSBAG_H      // Définition de CRC_H pour éviter l'inclusion multiple

#include <Arduino.h>
#include <Adafruit_BMP280.h>
#include <Wire.h>

extern Adafruit_BMP280 bmp;

void decode();
void C3Init(uint8_t Addr);
void bmpInit(uint8_t BMP_I2C_Addr);
int bmpReadPressure();
void C3SendConfig(uint8_t Addr);
void C3Read(uint8_t Addr, int &TempMSB, int &TempLSB, int &HumMSB, int &HumLSB);
void printAll(int T_MSB, int T_LSB, int H_MSB, int H_LSB, int pression);

unsigned long toUnixTimestamp(int year, int month, int day, int hour, int minute, int second);
bool isLeap(int year);



#endif  // Fin du bloc de garde d'inclusion
