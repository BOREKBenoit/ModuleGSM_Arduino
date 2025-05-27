#include "fonctionsBAG.h"
#include <Adafruit_BMP280.h>
#include <Wire.h>




// ======================== Partie SHT-C3 ========================

void C3Init(uint8_t Addr){

  Wire.begin(); //Initialisation de la librairie Wire et de l'I2C.

  Wire.beginTransmission(Addr); //Initialisation de la communication Arduino <----> SHTC3 avec l'adresse 0x70.
  Wire.write(0x80); //Envoi sur 2 octets la commande de réinitialisation système du capteur.
  Wire.write(0x5D);
   Wire.endTransmission();
}


void C3SendConfig(uint8_t Addr){

  Wire.beginTransmission(Addr); //Envoi de la commande de reveil avec le code 0x3517 sur 2 octets.
  Wire.write(0x35);
  Wire.write(0x17);
  Wire.endTransmission();
  delay(1000);
      Wire.beginTransmission(Addr); //Initialisation de la communication Arduino <----> SHTC3 avec l'adresse 0x70.
  Wire.write(0x80); //Envoi sur 2 octets la commande de réinitialisation système du capteur.
  Wire.write(0x5D);
   Wire.endTransmission();
  delay(1000);
  Wire.beginTransmission(Addr); //Envoi de la commande de conversion en "Normal mode" avec la température lu en premier.
  Wire.write(0x7C); //Envoi de 0x7CA2 sur 2 octets.
  Wire.write(0xA2);
  Wire.endTransmission();
  delay(1000);

 //Attente d'une seconde pour laisser le temps au capteur de convertir la température et l'humidité.

}

void C3Read(uint8_t Addr, int &T_MSB, int &T_LSB, int &H_MSB, int &H_LSB){
  float puissance = pow(2, 16);



  Wire.requestFrom(Addr, 6); //Envoi d'une requête de données sur 6 octets.
  // Deux octets sont pour la température, deux octets pour l'humidité et deux octets pour les checksum.
  //Serial.print("Octets en attente de lecture: "); //Affichage dans le moniteur série du nombre d'octets disponible à intégrer dans des variables.
  //Serial.println(Wire.available());
   T_MSB = Wire.read(); //Lecture du premier octet de température.
   T_LSB = Wire.read(); //Lecture du deuxième octet de température.
  int checksum1 = Wire.read(); // Attribution du checksum à une variable checksum1.
   H_MSB = Wire.read(); //Lecture du premier octet d'humidité.
   H_LSB = Wire.read(); //Lecture du deuxième octet d'humidité.
  int checksum2 = Wire.read(); // Attribution du checksum à une variable checksum2.


 Wire.beginTransmission(Addr); //Envoi sur 2 octets de la commande de mise en veille du capteur.
 Wire.write(0xB0); //Commande de mise en veille 0xB098.
 Wire.write(0x98);
 Wire.endTransmission();
 delay(1000);



}

// ======================== Partie BMP280 ========================


void bmpInit(uint8_t BMP_I2C_Addr){
  

  bmp.begin(BMP_I2C_Addr);

  bmp.setSampling(
    Adafruit_BMP280::MODE_FORCED,     /* Operating Mode. */
   Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
   Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
   Adafruit_BMP280::FILTER_X16,      /* Filtering. */
   Adafruit_BMP280::STANDBY_MS_500); 
 


}

int bmpReadPressure(){
  int pression;
  return pression = bmp.readPressure();
}

// ======================== Partie GSM ========================


void decode(){
    char sortie[240];
    String data; 


  
      delay(2000);
      while(Serial1.available()){
      data = data + Serial1.read();
    }
  
    int index = 0;
    for(size_t i=0; i < data.length(); i = i + 2){
      String pair = data.substring(i, i + 2);
      sortie[index] = pair.toInt();
      index = index + 1;
  
    }
        sortie[index] = '\0'; // Termine la chaîne
  
      Serial.println(sortie); // Affiche "AT OK"
  
      
  
  }

  // ======================== Calcul du timestamp ========================

  unsigned long toUnixTimestamp(int year, int month, int day, int hour, int minute, int second) {
    const int daysInMonth[] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
  
    unsigned long days = 0;
  
    for (int y = 1970; y < year; y++) {
      days += isLeap(y) ? 366 : 365;
    }
  
    for (int m = 1; m < month; m++) {
      days += daysInMonth[m - 1];
      if (m == 2 && isLeap(year)) days += 1;
    }
  
    days += day - 1;
  
    unsigned long totalSeconds = days * 86400UL;
    totalSeconds += hour * 3600UL;
    totalSeconds += minute * 60UL;
    totalSeconds += second;
  
    return totalSeconds - 3600;
  }

  bool isLeap(int year) {
    return ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
  }







