#include <Arduino.h>
#include <Adafruit_BMP280.h>
#include <Wire.h> //Importation de la bibliothèque Wire pour pouvoir communiquer avec le capteur.
#include "fonctionsBAG.h"


#define Addr 0x70 //Définition de la variable Addr avec la valeur héxadécimale 0x70.

// Partie BMP280
#define BMP_I2C_Addr 0x76
Adafruit_BMP280 bmp;



void setup() {
  delay(5000);

  Serial.begin(115200);//Initialisation de la communication Arduino <==> PC
  Serial1.begin(115200);//Initialisation de la communication Arduino <==> Module GSM

  while(!Serial) { //Attend que Serial soit initialisé.
    ;
  }
  while(!Serial1){ //Attend que Serial1 soit initialisé.
    ;
  }

  Serial.println("Partie 1 : test de la réponse du module");
  Serial1.println("AT");
  decode();


  Serial.println("Partie 2 : Débloquage de la SIM avec comme code pin \"0000\" ");
  Serial1.println("AT+CPIN=\"0000\"");
  decode();

//Partie SHT-C3

C3Init(Addr);
bmpInit(BMP_I2C_Addr);
}
 




void loop() {

  float pression = bmpReadPressure();
  C3SendConfig(Addr);

  float temperature, humidite;

  C3Read(Addr, temperature, humidite);



  Serial.println(); //Affichage de la température et de l'humidité dans le moniteur série.
  Serial.print("Température convertie : ");
  Serial.print(temperature);
  Serial.println("°C");
  Serial.println();
  Serial.print("Humidité convertie : ");
  Serial.print(humidite);
  Serial.println("%");
  Serial.println();
  Serial.print("Pression convertie : ");
  Serial.print(pression);
  Serial.println("mb");
  Serial.println();
  







}




