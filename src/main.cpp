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



bool isLeap(int year);
unsigned long toUnixTimestamp(int year, int month, int day, int hour, int minute, int second);


#define Addr 0x70 //Définition de la variable Addr avec la valeur héxadécimale 0x70.

// Partie BMP280
#define BMP_I2C_Addr 0x76
Adafruit_BMP280 bmp;

#define PIN_ANEMO 7

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

Serial.println("Démarrage du capteur de vent Peet Bros...");



}
 




void loop() {
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
  
  pulseISR();

  


  delay(1000);


 

// Création de la trame d'envoi



Serial.println("Création de la trame pour le serveur TTN.....");

String valeurFinale = String(timestamp, BIN);
Serial.println(valeurFinale);
int valFinale = valeurFinale.toInt();


Serial.println(valFinale, BIN);
int error;
modem.beginPacket();
modem.print(valFinale);
error = modem.endPacket(true);
if(error < 0){
  Serial.println("La trame n'a pas été transmise");
} else {
  Serial.println("La trame a été transmise");
}
delay(50000);



}




