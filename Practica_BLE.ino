#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#ifdef RGB_BUILTIN

#define DEVICE_NAME "ESP32-PEPITO"
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHAR_UUID    "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define BRIGHTNESS   20

BLEServer* pServer = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

// Variables para el parpadeo "sin bloqueo" (non-blocking)
unsigned long lastBlink = 0;
bool blinkState = false;

// Callback simplificado
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) { deviceConnected = true; };
    void onDisconnect(BLEServer* pServer) { deviceConnected = false; }
};

// Función única para efectos rápidos (bloqueantes) al cambiar de estado
void flashLED(int r, int g, int b, int count) {
  for (int i = 0; i < count; i++) {
    rgbLedWrite(RGB_BUILTIN, r, g, b); delay(100);
    rgbLedWrite(RGB_BUILTIN, 0, 0, 0); delay(100);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Iniciando BLE...");
  
  BLEDevice::init(DEVICE_NAME);
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);
  pService->createCharacteristic(CHAR_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY)->addDescriptor(new BLE2902());
  pService->start();

  // Configuración de Advertising condensada
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x12); // Compatibilidad iPhone
  BLEDevice::startAdvertising();
  
  Serial.println("Esperando cliente...");
}

void loop() {
  // 1. Lógica de CONEXIÓN (Nuevo dispositivo)
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
    Serial.println(">>> CONECTADO <<<");
    flashLED(0, 0, BRIGHTNESS, 5); // Parpadeo Azul Rápido
  }

  // 2. Lógica de DESCONEXIÓN
  if (!deviceConnected && oldDeviceConnected) {
    Serial.println(">>> DESCONECTADO <<<");
    flashLED(BRIGHTNESS, 0, 0, 5); // Parpadeo Rojo Rápido
    delay(500); 
    pServer->startAdvertising(); // Reiniciar visibilidad
    oldDeviceConnected = deviceConnected;
  }

  // 3. Estado MANTENIDO (Bucle principal)
  if (deviceConnected) {
    rgbLedWrite(RGB_BUILTIN, 0, 0, BRIGHTNESS); // Azul fijo
  } else {
    // Parpadeo Verde Lento (Esperando) usando millis para no bloquear
    if (millis() - lastBlink > 1000) {
      lastBlink = millis();
      blinkState = !blinkState;
      blinkState ? rgbLedWrite(RGB_BUILTIN, 0, BRIGHTNESS, 0) : rgbLedWrite(RGB_BUILTIN, 0, 0, 0);
    }
  }
  delay(10); // Estabilidad
}
#endif