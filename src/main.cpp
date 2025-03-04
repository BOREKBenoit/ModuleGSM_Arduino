#include <Arduino.h>
String data;
char sortie[120];


void setup() {
  
  Serial.begin(115200);
  Serial1.begin(115200);

  while(!Serial) {
    ;
  }
  while(!Serial1){
    ;
  }

  delay(2000);
  while(Serial1.available()){
    data = data + Serial1.read();
  }
  Serial.println(data);
  delay(2000);
  Serial.println("Communication avec le GSM");
  Serial.println("Envoie de la commande AT");

}

void loop() {

}

