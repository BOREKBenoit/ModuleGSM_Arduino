#include <Arduino.h>
#include <Adafruit_BMP280.h>
#include <Wire.h> //Importation de la bibliothèque Wire pour pouvoir communiquer avec le capteur.
#define Addr 0x70 //Définition de la variable Addr avec la valeur héxadécimale 0x70.

// Partie BMP280
#define BMP_I2C_Addr 0x76
Adafruit_BMP280 bmp;

//Partie GSM
String data; 
char sortie[120];

int reveil = 6;
int verifreveil = 7;
int reponse = 0;
void decode();  // Déclaration de la fonction


// Partie SHT-C3
float puissance = pow(2, 16);




void setup() {
// Partie GSM

 /* pinMode(reveil, OUTPUT);
  pinMode(verifreveil, INPUT);
  delay(2000);
  if(digitalRead(verifreveil)== HIGH){
    digitalWrite(reveil, LOW);
  } else {
    digitalWrite(reveil, HIGH);
    delay(2000);
    digitalWrite(reveil, LOW);
  }*/
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

  Wire.begin(); //Initialisation de la librairie Wire et de l'I2C.

  Wire.beginTransmission(Addr); //Initialisation de la communication Arduino <----> SHTC3 avec l'adresse 0x70.
  Wire.write(0x80); //Envoi sur 2 octets la commande de réinitialisation système du capteur.
  Wire.write(0x5D);
   Wire.endTransmission();

 
// Partie BMP280

  bmp.begin(BMP_I2C_Addr);

   bmp.setSampling(Adafruit_BMP280::MODE_FORCED,     /* Operating Mode. */
    Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
    Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
    Adafruit_BMP280::FILTER_X16,      /* Filtering. */
    Adafruit_BMP280::STANDBY_MS_500); 
  


}

void loop() {

  float pression = bmp.readPressure();
  Serial.print("Pression : ");
  Serial.print(pression);
  Serial.println(" Pa");
  pression = pression / 100;
  Serial.print(pression);
  Serial.println(" mb");


  Wire.beginTransmission(Addr); //Envoi de la commande de reveil avec le code 0x3517 sur 2 octets.
  Wire.write(0x35);
  Wire.write(0x17);
  Wire.endTransmission();
  delay(1000);
  Wire.beginTransmission(Addr); //Envoi de la commande de conversion en "Normal mode" avec la température lu en premier.
  Wire.write(0x7C); //Envoi de 0x7CA2 sur 2 octets.
  Wire.write(0xA2);
  Wire.endTransmission();
  delay(1000); //Attente d'une seconde pour laisser le temps au capteur de convertir la température et l'humidité.

  Wire.requestFrom(Addr, 6); //Envoi d'une requête de données sur 6 octets.
  // Deux octets sont pour la température, deux octets pour l'humidité et deux octets pour les checksum.
  //Serial.print("Octets en attente de lecture: "); //Affichage dans le moniteur série du nombre d'octets disponible à intégrer dans des variables.
  //Serial.println(Wire.available());
  int T_MSB = Wire.read(); //Lecture du premier octet de température.
  int T_LSB = Wire.read(); //Lecture du deuxième octet de température.
  int checksum1 = Wire.read(); // Attribution du checksum à une variable checksum1.
  int H_MSB = Wire.read(); //Lecture du premier octet d'humidité.
  int H_LSB = Wire.read(); //Lecture du deuxième octet d'humidité.
  int checksum2 = Wire.read(); // Attribution du checksum à une variable checksum2.

  float Temp = (T_MSB*255) + T_LSB; 
  Temp = -45 + 175 * (Temp/puissance); 

  //Conversion de l'humidité en valeur décimale.
  float Hum = (H_MSB*255) + H_LSB;
  Hum = 100 * (Hum/puissance);
 // Hum = 100 + Hum;

  Serial.println(); //Affichage de la température et de l'humidité dans le moniteur série.
  Serial.print("Température convertie : ");
  Serial.print(Temp);
  Serial.println("°C");
  Serial.println();
  Serial.print("Humidité convertie : ");
  Serial.print(Hum);
  Serial.println("%");
  Serial.println();




  Wire.beginTransmission(Addr); //Envoi sur 2 octets de la commande de mise en veille du capteur.
  Wire.write(0xB0); //Commande de mise en veille 0xB098.
  Wire.write(0x98);
  Wire.endTransmission();
  delay(1000);


}


void decode(){

  if(Serial1.available()){
    reponse = 1;

  } else {
    reponse = 0;
  }


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

