#include <WiFi.h>
#include <Tello.h>
#include <Wire.h>

// Parametri WiFi
const char * networkName = "TELLO-F16242";
const char * networkPswd = "";

// Indirizzo I2C del Sonar (deve coincidere con MYADDRESS nel file sonar.ino)
#define SONAR_ADDRESS 10

// Variabili per le coordinate ricevute
float Dx = 0;
float Dy = 0;
boolean connected = false;
boolean centered = false;

Tello tello;

// Struttura per facilitare la ricezione dei dati I2C
struct CoordinateData {
  float x;
  float y;
};

void setup() {
  Serial.begin(9600);
  
  // Inizializza I2C come Master
  Wire.begin(); 

  // Connessione WiFi al Tello
  connectToWiFi(networkName, networkPswd);
}

void loop() {
  if (connected) {
    // 1. Fase di decollo e posizionamento iniziale
    static bool initialized = false;
    if (!initialized) {
      tello.mon();            // Abilita Mission Pad
      delay(2000);
      tello.mdirection(2);    // Sensori verso il basso e frontali
      delay(2000);
      tello.takeoff();
      delay(5000);
      
      // Si alza a una quota di sicurezza (es. 60cm)
      tello.up(60);
      delay(4000);
      initialized = true;
    }

    // 2. Richiesta dati all'Arduino via I2C
    requestCoordinates();

    // 3. Logica di centratura
    // Se le coordinate sono diverse da ND (-1) e non siamo già centrati
    if (Dx != -1 && Dy != -1) {
      
      // Calcoliamo la distanza dallo zero (0,0)
      float distanceErr = sqrt(sq(Dx) + sq(Dy));

      Serial.print("Target trovato a X: "); Serial.print(Dx);
      Serial.print(" Y: "); Serial.println(Dy);

      // Soglia di tolleranza (es. 5cm)
      if (distanceErr < 5.0) {
        Serial.println("Target centrato! Atterraggio...");
        tello.land();
        connected = false; // Ferma il loop
      } else {
        // Logica di spostamento: tello.go(x, y, z, speed)
        // Nota: i segni di Dx e Dy dipendono da come è orientato il drone rispetto ai sensori
        // Se Dx > 0 e il drone deve tornare a 0, deve muoversi di -Dx.
        
        float moveX = -Dx; 
        float moveY = -Dy;
        
        Serial.print("Correzione in corso: "); Serial.print(moveX); Serial.print(", "); Serial.println(moveY);
        
        // Usiamo una velocità prudente (es. 20 cm/s)
        // Limita lo spostamento minimo (Tello accetta min 20cm in alcune versioni del firmware)
        tello.go(moveX, moveY, 0, 20); 
        
        delay(3000); // Aspetta che il movimento finisca
      }
    } else {
      Serial.println("Target non rilevato dai sensori...");
      delay(1000);
    }
  }
}

// Funzione per richiedere i dati all'Arduino
void requestCoordinates() {
  Wire.requestFrom(SONAR_ADDRESS, sizeof(float) * 2);
  
  if (Wire.available() >= sizeof(float) * 2) {
    byte* pX = (byte*)&Dx;
    for (int i = 0; i < sizeof(float); i++) pX[i] = Wire.read();
    
    byte* pY = (byte*)&Dy;
    for (int i = 0; i < sizeof(float); i++) pY[i] = Wire.read();
  }
}

void connectToWiFi(const char * ssid, const char * pwd) {
  Serial.println("Connecting to WiFi: " + String(ssid));
  WiFi.disconnect(true);
  WiFi.onEvent(WiFiEvent);
  WiFi.begin(ssid, pwd);
}

void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      Serial.print("WiFi connected! IP: ");
      Serial.println(WiFi.localIP());
      tello.init();
      connected = true;
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("WiFi lost connection");
      connected = false;
      break;
  }
}
