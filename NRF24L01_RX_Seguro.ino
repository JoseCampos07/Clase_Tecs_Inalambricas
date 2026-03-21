#include <SPI.h>
#include <RF24.h>
#include "mbedtls/aes.h"

#define CE_PIN    4 
#define CSN_PIN   5 
#define SCK_PIN   12 
#define MISO_PIN  13 
#define MOSI_PIN  11 
#define pinLed    18 

int brillo = 0; 
int distancia = 0; 
RF24 radio(CE_PIN, CSN_PIN); 
const byte direccion[6] = "00001"; 
bool encendido = false; 

// CLAVE DE SEGURIDAD (Debe ser idéntica a la del emisor)
const char* clave_segura = "SistemasI_2026_X"; 

void decrypt(unsigned char * cipherText, unsigned char * key, unsigned char * outputBuffer) {
  mbedtls_aes_context aes;
  mbedtls_aes_init(&aes);
  mbedtls_aes_setkey_dec(&aes, (const unsigned char*) key, 128);
  mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_DECRYPT, (const unsigned char*)cipherText, outputBuffer);
  mbedtls_aes_free(&aes);
}

void setup() { 
  Serial.begin(115200); 
  pinMode(pinLed, OUTPUT); 
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN); 

  if (!radio.begin()) { 
    Serial.println("NRF24 NO detectado"); 
    while (1); 
  } 

  radio.setChannel(69); 
  radio.setPALevel(RF24_PA_MAX); 
  radio.setDataRate(RF24_250KBPS); 
  radio.openReadingPipe(0, direccion); 
  radio.startListening(); 
  Serial.println("Receptor con Desencriptación Listo");
} 

void loop() { 
  if (radio.available()) { 
    unsigned char bufferRecibido[16];
    unsigned char bufferDescifrado[16];

    // Leer los 16 bytes cifrados
    radio.read(bufferRecibido, 16); 

    // Descifrar usando la clave
    decrypt(bufferRecibido, (unsigned char*)clave_segura, bufferDescifrado);

    // Extraer el entero de los primeros bytes del buffer
    memcpy(&distancia, bufferDescifrado, sizeof(distancia));
     
    Serial.print("Descifrado correctamente: "); 
    Serial.println(distancia); 
    encendido = !encendido; 
  } 

  brillo = map(distancia, 0, 50, 255, 0); 
  brillo = constrain(brillo, 0, 255); 
  
  if (distancia > 0) { 
    analogWrite(pinLed, brillo); 
  } 
  delay(10); 
}