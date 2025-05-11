#include <Wire.h>

#define trigPin 10
#define echoPin 11
#define MAXdistance 30 //in cm
#define MYADDRESS 10 //da cambiare con numero dell'arduino, per ordine non funzionalità

long duration;
int distance = -1;

void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  Wire.begin(MYADDRESS); //inizaliza protocollo I2C
  Wire.onRequest(I2C_DataSend); //a richiesta esegui funzione
}

void loop() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance= duration*0.034/2;
  if(distance > MAXdistance){
    distance = MAXdistance; //logica 1, se la distanza è oltre MAXdistance allora è MAXdistance
    //distance = -1;          //logica 2, se la distanza è oltre MAXdistance allora è -2
  }
  
}

void I2C_DataSend(){
  Wire.write(distance);
  Serial.print(MYADDRESS);
  Serial.print(" ha inviato ");
  Serial.print(distance);
  Serial.print("cm al Master\n");
}
