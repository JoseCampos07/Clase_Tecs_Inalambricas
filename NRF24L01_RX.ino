#include <SPI.h>
#include <RF24.h>

// Configuración de Pines NRF24L01 para ESP32-S3
#define CE_PIN   4
#define CSN_PIN  5
#define SCK_PIN  12
#define MISO_PIN 13
#define MOSI_PIN 11

RF24 radio(CE_PIN, CSN_PIN);
const byte direccion[6] = "00001";

void setup() {
  Serial.begin(115200);

  // Inicializamos el bus SPI con los pines de la S3
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN);

  if (!radio.begin()) {
    Serial.println("NRF24 NO detectado");
    #ifdef RGB_BUILTIN
      // Rojo si hay error de hardware
      rgbLedWrite(RGB_BUILTIN, RGB_BRIGHTNESS, 0, 0); 
    #endif
    while (1);
  }

  Serial.println("NRF24 detectado");
  
  // Configuración de radio original
  radio.setChannel(69);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  
  radio.openReadingPipe(0, direccion);
  radio.startListening();

  #ifdef RGB_BUILTIN
    // Azul tenue para indicar que está esperando señal
    rgbLedWrite(RGB_BUILTIN, 0, 0, 20); 
  #endif
}

void loop() {
  if (radio.available()) {
    char texto[32] = "";
    radio.read(&texto, sizeof(texto));
    
    Serial.print("Recibido: ");
    Serial.println(texto);

    #ifdef RGB_BUILTIN
      // Destello Verde al recibir cualquier señal
      rgbLedWrite(RGB_BUILTIN, 0, RGB_BRIGHTNESS, 0);
      delay(200); 
      // Volver a azul tenue
      rgbLedWrite(RGB_BUILTIN, 0, 0, 20);
    #endif
  }
}