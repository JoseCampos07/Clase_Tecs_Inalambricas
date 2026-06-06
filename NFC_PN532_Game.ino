#include <Wire.h>
#include <Adafruit_PN532.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// CREDENCIALES WiFi Y RUTA API
const char* ssid = "Redmi Note 11 Pro 5G";
const char* password = "12345678";
const char* apiUrlScore = "http://10.15.82.74:3000/api/puntuaciones";

// CONFIGURACIÓN PN532
#define I2C_SDA 4
#define I2C_SCL 5
Adafruit_PN532 nfc(I2C_SDA, I2C_SCL);

// ESTADOS DEL JUEGO
enum EstadoJuego { ESPERANDO_TARJETA, JUGANDO };
EstadoJuego estadoActual = ESPERANDO_TARJETA;

// FUNCIÓN PARA CONECTAR AL WIFI
void connectToWiFi() {
  Serial.print("Conectando a WiFi: ");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✓ Conectado a WiFi");
}

// FUNCIÓN PARA ENVIAR EL UID EN FORMATO JSON
bool checkCardAndSendToPC(String uidHex) { 
  Serial.println("USER_DATA:{\"id_rfid\":\""+uidHex+"\"}");
  return true;
}

// FUNCIÓN API PARA ENVIAR PUNTUACIÓN A SERVIDOR
bool updateScoreInDatabase(String jsonString) {
  HTTPClient http;
  http.begin(apiUrlScore);
  http.addHeader("Content-Type", "application/json");

  int httpCode = http.POST(jsonString); 
  http.end();
  
  if (httpCode == 200) {
    return true;
  } else {
    return false;
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  connectToWiFi();

  Wire.begin(I2C_SDA, I2C_SCL);
  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    while (1) delay(10);
  }

  nfc.SAMConfig();
}

void loop() {
  
  // ESTADO 1: Esperando tarjeta NFC
  if (estadoActual == ESPERANDO_TARJETA) {
    uint8_t uid[7]; 
    uint8_t uidLength;

    if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 500)) {
      String uidHex = "";
      for (uint8_t i = 0; i < uidLength; i++) {
        if (uid[i] < 0x10) uidHex += "0";
        uidHex += String(uid[i], HEX);
      }
      uidHex.toUpperCase();

      if (checkCardAndSendToPC(uidHex)) {
        estadoActual = JUGANDO;
      }
      delay(1000); 
    }
  }
  
  // ESTADO 2: Esperando el puntaje final desde WinUI 3
  else if (estadoActual == JUGANDO) {
    if (Serial.available() > 0) {
      String lineaDesdePC = Serial.readStringUntil('\n');
      lineaDesdePC.trim(); 

      if (lineaDesdePC.startsWith("UPDATE_SCORE:")) {
        String jsonPayload = lineaDesdePC.substring(13);

        if (updateScoreInDatabase(jsonPayload)) {
          estadoActual = ESPERANDO_TARJETA;
        } else {
          estadoActual = ESPERANDO_TARJETA; 
        }
      }
    }
  }
}