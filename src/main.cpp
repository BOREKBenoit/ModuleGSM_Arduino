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

#include <MKRWAN.h>
LoRaModem modem;

// === Paramètres de l'anémomètre ===
const float DIAMETRE_PO = 2.6;
const float DIAMETRE_CM = DIAMETRE_PO * 2.54;
const float RAYON_CM = DIAMETRE_CM / 2.0;
const float CIRCONFERENCE_CM = 2 * PI * RAYON_CM;

// === Débouncing ===
const unsigned long DEBOUNCE_MIN_DELTA_MS = 50; // 0.05 seconde

// === Pin de l'anémomètre ===
const byte PIN_ANEMO = 7;  // utilise une pin avec interruption (ex: pin 7)

// === Variables de mesure ===
volatile unsigned long t0 = 0;
volatile unsigned long tx = 0;
volatile bool firstPulseCaptured = false;
volatile bool newMeasurement = false;

void pulseISR() {
  unsigned long now = millis();

  static unsigned long lastPulse = 0;
  if (now - lastPulse < DEBOUNCE_MIN_DELTA_MS) return;
  lastPulse = now;

  if (!firstPulseCaptured) {
    t0 = now;
    firstPulseCaptured = true;
  } else {
    tx = now;
    newMeasurement = true;
    firstPulseCaptured = false;
  }
}



bool isLeap(int year);
unsigned long toUnixTimestamp(int year, int month, int day, int hour, int minute, int second);


#define Addr 0x70 //Définition de la variable Addr avec la valeur héxadécimale 0x70.

// Partie BMP280
#define BMP_I2C_Addr 0x76
Adafruit_BMP280 bmp;


String appEui = "0000000000000000";
String appKey = "E1510F7D6408ACA459E58EF86975727A";

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

  if (!modem.begin(EU868)) {
    Serial.println("Erreur lors du lancement du module");
    while (1) {}
}




  Serial.print("La version du module est : ");
  Serial.println(modem.version());
  Serial.print("Le numéro EUI du module est : ");
  Serial.println(modem.deviceEUI());

  int connected = modem.joinOTAA(appEui, appKey);
  if(!connected){
    Serial.println("Quelque chose n'a pas fonctionner avec la connection LoRa.");
    while(1);
  }
  modem.minPollInterval(60);

  Serial.println("Partie 1 : test de la réponse du module");
  Serial1.println("AT");
  decode();

/*
  Serial.println("Partie 2 : Débloquage de la SIM avec comme code pin \"0000\" ");
  Serial1.println("AT+CPIN=\"0000\"");
  decode();
  decode();
  decode();
  decode();

  Serial.println("Partie 3 : Configuration du mode d'envoi en texte ");
  Serial1.println("AT+CMGF=1");
  decode();

  Serial1.println("AT+CSCA?");
  decode();


  Serial.println("Partie 4 : Envoi du niveau d'alerte ");
  Serial1.println("AT+CMGS=\"+33789216632\""); //+33 765836249
  decode();
  Serial1.println("Message de test");
  delay(100);
  Serial1.write(26);
  decode();*/

//Partie SHT-C3

C3Init(Addr);
bmpInit(BMP_I2C_Addr);

pinMode(PIN_ANEMO, INPUT_PULLUP);
attachInterrupt(digitalPinToInterrupt(PIN_ANEMO), pulseISR, FALLING);

Serial.println("Démarrage de l'anémomètre...");




}
 




void loop() { /* ===== Debug Module GSM ====== */
  Serial1.println("AT");
  decode();
  delay(1000);
  Serial1.println("AT");
  decode();
  delay(1000);
  Serial1.println("AT");
  decode();
  delay(1000);
  Serial1.println("AT");
  decode();
  delay(1000);
  Serial1.println("AT");
  decode();
  delay(1000);
  Serial1.println("AT");
  decode();
  delay(1000);

  if (newMeasurement) {
    noInterrupts(); // on empêche les interruptions pendant le calcul
    unsigned long deltaT_ms = tx - t0;
    newMeasurement = false;
    interrupts();

    if (deltaT_ms > 0) {
      float deltaT_s = deltaT_ms / 1000.0;
      float vitesse_cm_s = CIRCONFERENCE_CM / deltaT_s;
      float vitesse_m_s = vitesse_cm_s / 100.0;
      float vitesse_km_h = vitesse_m_s * 3.6;

      Serial.print("Δt = ");
      Serial.print(deltaT_s);
      Serial.print(" s | Vitesse du vent : ");
      Serial.print(vitesse_m_s);
      Serial.print(" m/s (");
      Serial.print(vitesse_km_h);
      Serial.println(" km/h)");
    }
  }
  

  Serial1.println("AT+CCLK?");
  delay(1000);

  char sortie[120];
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
    Serial.println("====");
    int jour;
    int mois;
    int annee;
    int heure;
    int minute;
    int seconde;

    if (isdigit(sortie[19]) && isdigit(sortie[20])) {
      jour = (sortie[19] - '0') * 10 + (sortie[20] - '0'); // Convertir '4''2' → 42
    }
    if (isdigit(sortie[22]) && isdigit(sortie[23])) {
      mois = (sortie[22] - '0') * 10 + (sortie[23] - '0'); // Convertir '4''2' → 42
    }
    if (isdigit(sortie[25]) && isdigit(sortie[26])) {
    annee = 2000 + (sortie[25] - '0') * 10 + (sortie[26] - '0'); // Convertir '4''2' → 42
    }
    if (isdigit(sortie[28]) && isdigit(sortie[29])) {
      heure = (sortie[28] - '0') * 10 + (sortie[29] - '0'); // Convertir '4''2' → 42
    }
    if (isdigit(sortie[31]) && isdigit(sortie[32])) {
      minute = (sortie[31] - '0') * 10 + (sortie[32] - '0'); // Convertir '4''2' → 42
    }
    if (isdigit(sortie[34]) && isdigit(sortie[35])) {
      seconde = (sortie[34] - '0') * 10 + (sortie[35] - '0'); // Convertir '4''2' → 42
    }


    unsigned long timestamp = toUnixTimestamp(annee, mois, jour, heure, minute, seconde);

    Serial.print("Timestamp UNIX : ");
    Serial.println(timestamp);



  

  int pression = bmpReadPressure();
  C3SendConfig(Addr);

  int temperatureMSB, temperatureLSB, humiditeMSB, humiditeLSB;
  C3Read(Addr, temperatureMSB,temperatureLSB, humiditeMSB, humiditeLSB);

  Serial.print("Temperature MSB : ");
  Serial.println(temperatureMSB);
  Serial.print("Temperature LSB : ");
  Serial.println(temperatureLSB);
  Serial.print("Humidite MSB : ");
  Serial.println(humiditeMSB);
  Serial.print("Humidite LSB : ");
  Serial.println(humiditeLSB);
  Serial.print("Pression : ");
  Serial.println(pression);
  





  delay(1000);


 

// Création de la trame d'envoi



Serial.println("Création de la trame pour le serveur TTN.....");
//========== Convertion quand c'est négatif ==========

String STemperature_MSB = String(temperatureMSB,BIN);
String STemperature_LSB = String(temperatureLSB, BIN);
String SHumidite_MSB = String(humiditeMSB, BIN);
String SHumidite_LSB = String(humiditeLSB, BIN);

if(temperatureMSB < 0){
  STemperature_MSB = "";
  byte condensed = (byte)temperatureMSB;
for (int i = 7; i >= 0; i--) {
  STemperature_MSB += bitRead(condensed, i);  // lit bit par bit de gauche à droite
}
} 

if(temperatureLSB < 0){
  STemperature_LSB = "";
  byte condensed = (byte)temperatureLSB;
for (int i = 7; i >= 0; i--) {
  STemperature_LSB += bitRead(condensed, i);  // lit bit par bit de gauche à droite
}
}

if(humiditeLSB < 0){
  SHumidite_LSB = "";
  byte condensed = (byte)humiditeLSB;
for (int i = 7; i >= 0; i--) {
  SHumidite_LSB += bitRead(condensed, i);  // lit bit par bit de gauche à droite
}
}


if(humiditeMSB < 0){
  SHumidite_MSB = "";
  byte condensed = (byte)humiditeMSB;
for (int i = 7; i >= 0; i--) {
  SHumidite_MSB += bitRead(condensed, i);  // lit bit par bit de gauche à droite
}
}




String STimestamp = String(timestamp, BIN);
Serial.println(STimestamp);
Serial.println(STemperature_MSB);
Serial.println(STemperature_LSB);
Serial.println(SHumidite_MSB);
Serial.println(SHumidite_LSB);



int len = STemperature_MSB.length();
int nextMultiple = ((len + 7) / 8) * 8;  // arrondi au multiple de 8 supérieur
while (STemperature_MSB.length() < nextMultiple) {
  STemperature_MSB = "0" + STemperature_MSB;
}




 len = STemperature_LSB.length();
 nextMultiple = ((len + 7) / 8) * 8;  // arrondi au multiple de 8 supérieur
while (STemperature_LSB.length() < nextMultiple) {
  STemperature_LSB = "0" + STemperature_LSB;
}



 len = SHumidite_LSB.length();
 nextMultiple = ((len + 7) / 8) * 8;  // arrondi au multiple de 8 supérieur
while (SHumidite_LSB.length() < nextMultiple) {
  SHumidite_LSB = "0" + SHumidite_LSB;
}



 len = SHumidite_MSB.length();
 nextMultiple = ((len + 7) / 8) * 8;  // arrondi au multiple de 8 supérieur
while (SHumidite_MSB.length() < nextMultiple) {
  SHumidite_MSB = "0" + SHumidite_MSB;
}


Serial.println("=======");

Serial.println(STemperature_MSB);
Serial.println(STemperature_LSB);
Serial.println(SHumidite_MSB);
Serial.println(SHumidite_LSB);

Serial.println(STimestamp+STemperature_MSB+STemperature_LSB+SHumidite_MSB+SHumidite_LSB);

int error;
modem.beginPacket();
modem.print(STimestamp);
error = modem.endPacket(true);
if(error < 0){
  Serial.println("La trame n'a pas été transmise");
} else {
  Serial.println("La trame a été transmise");
}
delay(10000);



}




