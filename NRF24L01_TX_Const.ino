#include <SPI.h>
#include <RF24.h>

#define CE_PIN 4
#define CSN_PIN 5
#define echo 21
#define trigger 22

int distancia;
RF24 radio(CE_PIN, CSN_PIN);
const byte direccion[6] = "00001";

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

  Serial.println("Transmisor listo (ESP32)");
}

void loop() {
  distancia = 0.01723 * readUltrasonicDistance(trigger, echo);

  if (radio.write(&distancia, sizeof(distancia))){
    Serial.print("Distancia enviada:");
  } else {
    Serial.println("Error");
  }

  Serial.println(distancia);
}