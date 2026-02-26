#include <EEPROM.h>
#include <Wire.h>

#define MYADDRESS 10 // da cambiare con numero dell'arduino, per ordine non funzionalità
#define ND -1

// EEPROM data
#define EPR_Ax 0
#define EPR_Ay 4
#define EPR_Bx 8
#define EPR_By 12
#define EPR_Cx 16
#define EPR_Cy 20
#define EPR_Max 24

// per setup
unsigned long timeout = 10000; // 10 secondi
unsigned long startTime;

// Definizione dei pin trig e echo per ciascun sensore
const int trigPins[3] = {2, 4, 6};
const int echoPins[3] = {3, 5, 7};

// Array delle distanze
int distances[3] = {ND, ND, ND};
int MAXdistance = 80; // cm

// posizione dei sensori
float Ax = 0, Ay = 20.0;
float Bx = 17.32, By = -10;
float Cx = -17.32, Cy = -10;

String input = "";

// errore
bool error = true;
bool enter = false;

// coordinate punto
float x, y; // in elaborazione
float Dx, Dy; // da inviare

// Funzione per leggere la distanza di un singolo sensore
int readDistance(int trigPin, int echoPin) {
  long duration;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH, 30000); // timeout 30ms
  if (duration == 0) {
    error = true;
    return ND; // no echo
  }

  int dist = duration * 0.034 / 2;
  if (dist > MAXdistance) {
    error = true;
    return ND;
  }
  return dist;
}

// chiedi le coordinate dei sensori
void coordinateSensori() {
  Serial.print("Ax = ");
  input = readLine();
  input.trim();
  Ax = input.toFloat();

  Serial.print("Ay = ");
  input = readLine();
  input.trim();
  Ay = input.toFloat();

  Serial.print("Bx = ");
  input = readLine();
  input.trim();
  Bx = input.toFloat();

  Serial.print("By = ");
  input = readLine();
  input.trim();
  By = input.toFloat();

  Serial.print("Cx = ");
  input = readLine();
  input.trim();
  Cx = input.toFloat();

  Serial.print("Cy = ");
  input = readLine();
  input.trim();
  Cy = input.toFloat();

  Serial.print("MAXdistance = ");
  input = readLine();
  input.trim();
  MAXdistance = input.toFloat();
}

// funzione per mandare i dati a ESP32 DevKit
void I2C_DataSend() {
  Serial.print(MYADDRESS);
  Serial.print(" ha inviato X: ");
  Serial.print(Dx);
  Wire.write((byte *)&Dx, sizeof(Dx));
  Serial.print(" e Y: ");
  Serial.print(Dy);
  Wire.write((byte *)&Dy, sizeof(Dy));
  Serial.println(" al Master\n");
}

String readLine() {
  String datoricevuto = "";
  char c;

  while (Serial.available()) Serial.read(); // svuota buffer
  while (!Serial.available()); // aspetta input
  delay(10);
  while (Serial.available() > 0) {
    c = Serial.read();
    if (c == '\n') {
      break;
    } else {
      datoricevuto += c;
    }
  }
  return datoricevuto;
}

void setup() {
  Serial.begin(9600);
  Wire.begin(MYADDRESS);
  Wire.onRequest(I2C_DataSend);

  for (int i = 0; i < 3; i++) {
    pinMode(trigPins[i], OUTPUT);
    pinMode(echoPins[i], INPUT);
  }

  EEPROM.get(EPR_Ax, Ax);
  EEPROM.get(EPR_Ay, Ay);
  EEPROM.get(EPR_Bx, Bx);
  EEPROM.get(EPR_By, By);
  EEPROM.get(EPR_Cx, Cx);
  EEPROM.get(EPR_Cy, Cy);
  EEPROM.get(EPR_Max, MAXdistance);

  Serial.println("Premi un tasto entro 10 secondi per entrare in modalità setup...");
  startTime = millis();
  while (Serial.available()) Serial.read();
  while (millis() - startTime < timeout && !enter) {
    if (Serial.available() > 0) {
      Serial.println("Entrato in modalità setup.");
      Serial.println("Coordinate dei Sensori:");
      Serial.print("Ax = "); Serial.print(Ax); Serial.print(" Ay = "); Serial.println(Ay);
      Serial.print("Bx = "); Serial.print(Bx); Serial.print(" By = "); Serial.println(By);
      Serial.print("Cx = "); Serial.print(Cx); Serial.print(" Cy = "); Serial.println(Cy);
      Serial.print("Distanza Massima = "); Serial.println(MAXdistance);
      Serial.println("Inserisci 0 per uscire, 1 per sceglierli provvisoriamente, 2 per modificare la EEPROM: ");

      while (Serial.available()) Serial.read();
      input = readLine();
      input.trim();

      switch (input.toInt()) {
        case 1:
          coordinateSensori();
          break;
        case 2:
          coordinateSensori();
          EEPROM.put(EPR_Ax, Ax);
          EEPROM.put(EPR_Ay, Ay);
          EEPROM.put(EPR_Bx, Bx);
          EEPROM.put(EPR_By, By);
          EEPROM.put(EPR_Cx, Cx);
          EEPROM.put(EPR_Cy, Cy);
          EEPROM.put(EPR_Max, MAXdistance);
          break;
        case 2025:
          Serial.print("Aldo Aldo Dove sei finito?\nNel baule verso l′infinito\nLe Brigate Rosse ti danno la caccia\nMa tu sei un mago e non temi la minaccia");
          Serial.println("https://www.youtube.com/watch?v=x2P2D--dEPc");
        default:
          break;
      }
      enter = true;
    }
  }
  Serial.println("Avvio programma principale.");
}

void loop() {
  error = false;
  for (int i = 0; i < 3; i++) {
    distances[i] = readDistance(trigPins[i], echoPins[i]);
    if (distances[i] == ND) error = true;
      Serial.print(i);
      Serial.print(" = ");
      Serial.println(distances[i]);
    delay(50);
  }

  if (!error) {
    float A = 2 * (Bx - Ax);
    float B = 2 * (By - Ay);
    float C = sq(distances[0]) - sq(distances[1]) - sq(Ax) + sq(Bx) - sq(Ay) + sq(By);

    float D = 2 * (Cx - Bx);
    float E = 2 * (Cy - By);
    float F = sq(distances[1]) - sq(distances[2]) - sq(Bx) + sq(Cx) - sq(By) + sq(Cy);

    float denominator = A * E - B * D;

    if (denominator != 0) {
      x = (C * E - B * F) / denominator;
      y = (A * F - C * D) / denominator;
      Dx = x;
      Dy = y;
    } else {
      x = ND;
      y = ND;
    }
  } else {
    x = ND;
    y = ND;
  }
  Serial.println(MYADDRESS);
  Serial.print(" ha inviato X: ");
  Serial.print(Dx);
  Serial.print(" e Y: ");
  Serial.print(Dy);
  Serial.println(" al Master\n");
  delay(2000);
}
