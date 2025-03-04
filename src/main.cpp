#include <Arduino.h>
String data; 
char sortie[120];

int reveil = 6;
int verifreveil = 7;
int reponse = 0;



void decode();  // Déclaration de la fonction



void setup() {

  pinMode(reveil, OUTPUT);
  pinMode(verifreveil, INPUT);
  delay(1000);
  if(digitalRead(verifreveil)== HIGH){
    digitalWrite(reveil, LOW);
  } else {
    digitalWrite(reveil, HIGH);
    delay(1000);
    digitalWrite(reveil, LOW);
  }
  delay(2000);

  
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

 

  


}

void loop() {

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

