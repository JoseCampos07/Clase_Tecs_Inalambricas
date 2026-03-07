#include <SPI.h> 
#include <RF24.h> 
#define CE_PIN 4 
#define CSN_PIN 5 
RF24 radio(CE_PIN, CSN_PIN); 
const byte direccion[6] = "00001"; 

void setup() { 
  Serial.begin(115200); 
  if (!radio.begin()) { 
    Serial.println("NRF24 NO detectado"); 
    while (1); 
  }
  
  radio.setChannel(69); 
  radio.setPALevel(RF24_PA_MAX); 
  radio.setDataRate(RF24_250KBPS); 
  radio.openWritingPipe(direccion); 
  radio.stopListening(); 
  
  Serial.println("Transmisor listo (ESP32)"); 
} 

void loop() { 
  const char texto[] = "Pago mandado"; 
  bool ok = radio.write(&texto, sizeof(texto)); 
  if (ok) { 
    Serial.println("Enviado correctamente"); 
  } else { 
    Serial.println("Error al enviar"); 
  } 
  
  delay(1000); 
}