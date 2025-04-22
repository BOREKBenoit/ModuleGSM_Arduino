/* 
  Capture de la température/humidité/vitesse du vent et initialisation du module GSM.
  
  Programme Basique sur Arduino MKR WAN 1310.
  IDE Visual Studio Code 1.98.0

  Constituants :
    - Arduino MKR WAN 1310
    - SHT-C3
    - BMP280
    - Module GSM Waveshare
    - Anémomètre PEETBROS

    Version 1 : 11/03/2025
    Benoit Borek
  */



#include <Arduino.h>
#include <Adafruit_BMP280.h>
#include <Wire.h> //Importation de la bibliothèque Wire pour pouvoir communiquer avec le capteur.
#include "fonctionsBAG.h"




#define Addr 0x70 //Définition de la variable Addr avec la valeur héxadécimale 0x70.

// Partie BMP280
#define BMP_I2C_Addr 0x76
Adafruit_BMP280 bmp;

#define REED_PIN 7  

volatile int compteur_impulsions = 0;
volatile unsigned long last_interrupt_time = 0;
const double FACTEUR_KMH = 0.25;  // Facteur ajusté
const int DEBOUNCE_TIME = 10; 



void impulsion_detectee() {

  unsigned long interrupt_time = millis();
  if (interrupt_time - last_interrupt_time > DEBOUNCE_TIME) {
      compteur_impulsions++;
      last_interrupt_time = interrupt_time;
  }

}






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


pinMode(7, INPUT);  // Pas de INPUT_PULLUP sur MKR WAN !
attachInterrupt(digitalPinToInterrupt(7), impulsion_detectee, FALLING);
}
 




void loop() {

  float pression = bmpReadPressure();
  C3SendConfig(Addr);

  float temperature, humidite;
  C3Read(Addr, temperature, humidite);

  double vitesse_vent_kmh = compteur_impulsions * FACTEUR_KMH * 12;  // 12 pour passer à l'heure

  printAll(temperature, humidite, pression, vitesse_vent_kmh);

  compteur_impulsions = 0;
  delay(1000);
 

// Calcul niveau de grêle et envoi par SMS.





}




