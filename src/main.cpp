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


float latitude;
float longitude;

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

float vitesse_km_h = 0.0; // Vitesse du vent en km/h

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





  Serial1.println("AT+CPIN?");
  decode();
  decode();
  decode();
  decode();
/*
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
 




void loop() { 

  if (newMeasurement) {
    noInterrupts(); // on empêche les interruptions pendant le calcul
    unsigned long deltaT_ms = tx - t0;
    newMeasurement = false;
    interrupts();

    if (deltaT_ms > 0) {
      float deltaT_s = deltaT_ms / 1000.0;
      float vitesse_cm_s = CIRCONFERENCE_CM / deltaT_s;
      float vitesse_m_s = vitesse_cm_s / 100.0;
      vitesse_km_h = vitesse_m_s * 3.6;
    
      // Affichage classique
      Serial.print("Δt = ");
      Serial.print(deltaT_s);
      Serial.print(" s | Vitesse du vent : ");
      Serial.print(vitesse_m_s);
      Serial.print(" m/s (");
      Serial.print(vitesse_km_h);
      Serial.println(" km/h)");
    


    

    }
    
  }
  
  float vitesseLimitee = min(vitesse_km_h, 63.75);
    
  // Séparer la partie entière et la partie décimale
  uint8_t partieEntiere = (uint8_t)vitesseLimitee;           // 0–63
  float partieDecimale = vitesseLimitee - partieEntiere;

  // Convertir la partie décimale en 2 bits (0.00, 0.25, 0.50, 0.75)
  uint8_t decimal2bits = round(partieDecimale * 4.0);         // 0 à 3

  // Fusionner les deux dans un octet
  uint8_t vitesse8bit = (partieEntiere << 2) | (decimal2bits & 0b11);

  // Affichage binaire sur 8 bits
  String Svitesse = String(vitesse8bit, BIN);
  while (Svitesse.length() < 8) {
    Svitesse = "0" + Svitesse;
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
    int jour = 0;
    int mois =0;
    int annee=0;
    int heure=0;
    int minute=0;
    int seconde=0;

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



  

    int pression = bmpReadPressure(); // Exemple: 101325 (en Pa)

    // Convertir en hPa
    float pression_hPa = pression / 100.0;
    
    // Plage typique : 300 à 1100 hPa
    pression_hPa = constrain(pression_hPa, 300.0, 1100.0);
    
    // Encodage sur 8 bits
    uint8_t pression8bit = round((pression_hPa - 300.0) * 255.0 / 800.0);
    
    // Conversion en binaire (8 bits)
    String Spression = String(pression8bit, BIN);
    
    // Complétion à 8 bits
    while (Spression.length() < 8) {
      Spression = "0" + Spression;
    }
    


    
  C3SendConfig(Addr);

  int temperatureMSB, temperatureLSB, humiditeMSB, humiditeLSB;
  C3Read(Addr, temperatureMSB, temperatureLSB, humiditeMSB, humiditeLSB);
  
  // -------- Conversion brute 16 bits --------
  uint16_t rawTemperature = (temperatureMSB << 8) | temperatureLSB;
  uint16_t rawHumidity = (humiditeMSB << 8) | humiditeLSB;
  
  // -------- Formules SHT-C3 datasheet --------
  // Température en °C
  float temperatureC = -45 + 175 * (rawTemperature / 65535.0);
  
  // Humidité en %RH
  float humidityRH = 100 * (rawHumidity / 65535.0);
  
  // -------- Limiter les plages utiles --------
  temperatureC = constrain(temperatureC, -40.0, 125.0);  // plage typique
  humidityRH   = constrain(humidityRH, 0.0, 100.0);      // plage normale
  
  // -------- Encodage sur 8 bits --------
  // Température : on mappe -40 à 125 °C → 0 à 255
  uint8_t temp8bit = round((temperatureC + 40.0) * 255.0 / 165.0);
  
  // Humidité : on mappe 0 à 100 % → 0 à 255
  uint8_t hum8bit = round(humidityRH * 255.0 / 100.0);
  
  // -------- Affichage des résultats --------
  Serial.print("Température °C : ");
  Serial.println(temperatureC);
  Serial.print("Température encodée (8 bits) : ");
  Serial.println(temp8bit, BIN);
  
  Serial.print("Humidité %RH : ");
  Serial.println(humidityRH);
  Serial.print("Humidité encodée (8 bits) : ");
  Serial.println(hum8bit, BIN);

// ----- Envoi alerte -----
  if (temperatureC > 25.0 || humidityRH > 50.0) {
 //envoieNivUn();
  }
  


  Serial.print("Pression encodée sur 8 bits : ");
  Serial.println(Spression);
  Serial.print("Vitesse encodée sur 8 bits : ");
  Serial.println(Svitesse);

  delay(1500);


  C3SendConfig(Addr);

  delay(60000);
// =========GPS==========
Serial.println("Activation du GPS");
Serial1.println("AT+CGNSPWR=1");
delay(1000); 
decode();
Serial.println("Demande d'informations GPS");


 // Envoi de la commande GPS
Serial1.println("AT+CGNSINF");
delay(500); // temps de réponse du module

// Lecture de la réponse
String ligneGPS = "";
unsigned long timeout = millis();
while (millis() - timeout < 1000) {
  if (Serial1.available()) {
    ligneGPS = Serial1.readStringUntil('\n');
    if (ligneGPS.startsWith("+CGNSINF:")) {
      break;
    }
  }
}

if (ligneGPS.startsWith("+CGNSINF:")) {
  // Exemple : +CGNSINF: 1,1,20240527173201.000,48.8566,2.3522,35.0,...
  int indexDeuxPoints = ligneGPS.indexOf(':');
  String data = ligneGPS.substring(indexDeuxPoints + 1);
  data.trim(); // Supprime les espaces

  // Découpage par virgule
  int idx1 = data.indexOf(',');                  // GNSS run status
  int idx2 = data.indexOf(',', idx1 + 1);        // Fix status
  int idx3 = data.indexOf(',', idx2 + 1);        // UTC time
  int idx4 = data.indexOf(',', idx3 + 1);        // Latitude
  int idx5 = data.indexOf(',', idx4 + 1);        // Longitude

  String latStr = data.substring(idx3 + 1, idx4);
  String lonStr = data.substring(idx4 + 1, idx5);

  latitude = latStr.toFloat();
  longitude = lonStr.toFloat();

  Serial.print("Latitude : ");
  Serial.println(latitude, 6);
  Serial.print("Longitude : ");
  Serial.println(longitude, 6);
} else {
  Serial.println(" Pas de réponse +CGNSINF trouvée");
}
float minLat = -90.0;
float maxLat = 90.0;

// Étape 1 : Mapper la latitude dans l'intervalle 0-255
uint8_t encodedLat = (uint8_t)((latitude - minLat) * 255.0 / (maxLat - minLat));

// Afficher en binaire
Serial.print("Latitude encodée sur 1 octet : ");
Serial.println(encodedLat, BIN);

float minLong = -90.0;
float maxLong = 90.0;

// Étape 1 : Mapper la latitude dans l'intervalle 0-255
uint8_t encodedLong = (uint8_t)((longitude - minLong) * 255.0 / (maxLong - minLong));

// Afficher en binaire
Serial.print("Longitude encodée sur 1 octet : ");
Serial.println(encodedLong, BIN);



// Création de la trame d'envoi



Serial.println("Création de la trame pour le serveur TTN.....");
//========== Convertion quand c'est négatif ==========

String STemperature_MSB = String(hum8bit,BIN);

String SHumidite_MSB = String(hum8bit, BIN);



if(temperatureMSB < 0){
  STemperature_MSB = "";
  byte condensed = (byte)temperatureMSB;
for (int i = 7; i >= 0; i--) {
  STemperature_MSB += bitRead(condensed, i);  // lit bit par bit de gauche à droite
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




int len = STemperature_MSB.length();
int nextMultiple = ((len + 7) / 8) * 8;  // arrondi au multiple de 8 supérieur
while (STemperature_MSB.length() < nextMultiple) {
  STemperature_MSB = "0" + STemperature_MSB;
}



 len = SHumidite_MSB.length();
 nextMultiple = ((len + 7) / 8) * 8;  // arrondi au multiple de 8 supérieur
while (SHumidite_MSB.length() < nextMultiple) {
  SHumidite_MSB = "0" + SHumidite_MSB;
}




Serial.println("=======");
Serial.println("Temp : ");
Serial.println(STemperature_MSB);

Serial.println("Hum : ");
Serial.println(SHumidite_MSB);

Serial.println("Pression : ");
Serial.println(Spression);
Serial.println("Vitesse : ");
Serial.println(Svitesse);

Serial.println(STimestamp+STemperature_MSB+SHumidite_MSB+Spression+Svitesse);

int error;
modem.beginPacket();
modem.print(STimestamp+STemperature_MSB+SHumidite_MSB+Spression+Svitesse);
error = modem.endPacket(true);
if(error < 0){
  Serial.println("La trame n'a pas été transmise");
} else {
  Serial.println("La trame a été transmise");
}





}




