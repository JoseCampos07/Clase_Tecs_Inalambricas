#include <SPI.h>
#include <MFRC522.h>

// Definimos los pines
#define SS_PIN  5   // SDA
#define RST_PIN 9   // RST

MFRC522 rfid(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(115200);
  SPI.begin(6, 8, 7); // SCK=6, MISO=8, MOSI=7
  rfid.PCD_Init();
  Serial.println("Acerque una tarjeta RFID...");
}

void loop() {
  // Verifica si hay una tarjeta presente
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  Serial.println("=== Tarjeta detectada ===");

  // Mostramos el UID en formato hexadecimal para el programa en Windows
  Serial.print("UID: ");
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
    Serial.print(rfid.uid.uidByte[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  // Detener comunicación con la tarjeta
  rfid.PICC_HaltA();
}