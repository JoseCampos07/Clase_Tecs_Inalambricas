#include <SPI.h>
#include <RF24.h>
#include "mbedtls/aes.h"

#define CE_PIN 10 
#define CSN_PIN 9 
#define echo 5 
#define trigger 4

int distancia; 
RF24 radio(CE_PIN, CSN_PIN); 
const byte direccion[6] = "00001"; 

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
  
  // Inicialización del bus SPI con los pines 11, 12, 13
  SPI.begin(12, 13, 11); // SCK, MISO, MOSI

  if (!radio.begin()) { 
    Serial.println("NRF24 NO detectado"); 
    while (1); 
  } 
  radio.setChannel(69); 
  radio.setPALevel(RF24_PA_MAX); 
  radio.setDataRate(RF24_250KBPS); 
  radio.openWritingPipe(direccion); 
  radio.stopListening(); 
  Serial.println("Transmisor S3 Listo en Pines 4 y 5"); 
} 

void loop() { 
    distancia = 0.01723 * readUltrasonicDistance(trigger, echo); 
    
    unsigned char bufferEntrada[16] = {0}; 
    unsigned char bufferCifrado[16] = {0};
    
    memcpy(bufferEntrada, &distancia, sizeof(distancia));

    encrypt(bufferEntrada, (unsigned char*)clave_segura, bufferCifrado);

    if (radio.write(bufferCifrado, 16)){ 
      Serial.print("Enviado (Cifrado): "); 
      Serial.println(distancia);
    } else { 
      Serial.println("Error de envío"); 
    } 
    delay(50); 
}