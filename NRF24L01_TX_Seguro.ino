#include <SPI.h>
#include <RF24.h>
#include "mbedtls/aes.h"

#define CE_PIN 4 
#define CSN_PIN 5 
#define echo 21 
#define trigger 22    

int distancia; 
RF24 radio(CE_PIN, CSN_PIN); 
const byte direccion[6] = "00001"; 

// CLAVE DE SEGURIDAD (Debe tener exactamente 16 caracteres)
const char* clave_segura = "SistemasI_2026_X"; 

void encrypt(unsigned char * plainText, unsigned char * key, unsigned char * outputBuffer) {
  mbedtls_aes_context aes;
  mbedtls_aes_init(&aes);
  mbedtls_aes_setkey_enc(&aes, (const unsigned char*) key, 128);
  mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_ENCRYPT, (const unsigned char*)plainText, outputBuffer);
  mbedtls_aes_free(&aes);
}

long readUltrasonicDistance(int triggerPin, int echoPin){ 
  pinMode(triggerPin, OUTPUT); 
  digitalWrite(triggerPin, LOW); 
  delayMicroseconds(2); 
  digitalWrite(triggerPin, HIGH); 
  delayMicroseconds(10); 
  digitalWrite(triggerPin, LOW); 
  pinMode(echoPin, INPUT); 
  return pulseIn(echoPin, HIGH); 
} 

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
  Serial.println("Transmisor con Encriptación Listo"); 
} 

void loop() { 
    distancia = 0.01723 * readUltrasonicDistance(trigger, echo); 
    
    // Preparar buffers para AES (Bloques de 16 bytes)
    unsigned char bufferEntrada[16] = {0}; 
    unsigned char bufferCifrado[16] = {0};
    
    // Copiamos la distancia al buffer de entrada
    memcpy(bufferEntrada, &distancia, sizeof(distancia));

    // Cifrar la información
    encrypt(bufferEntrada, (unsigned char*)clave_segura, bufferCifrado);

    // Enviamos el bloque cifrado de 16 bytes
    if (radio.write(bufferCifrado, 16)){ 
      Serial.print("Enviado (Cifrado): "); 
      Serial.println(distancia);
    } else { 
      Serial.println("Error de envío"); 
    } 
    delay(50); 
}