#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#ifdef RGB_BUILTIN

#define DEVICE_NAME "ESP32-PEPITO"
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHAR_UUID    "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define BRIGHTNESS   50 // Aumentado un poco para que se vea mejor

BLEServer* pServer = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

// Variables de control de estado
unsigned long lastBlink = 0;
unsigned long connectionTime = 0; // Para medir cuánto tiempo lleva conectado
bool blinkState = false;
bool manualControl = false; // Indica si ya podemos cambiar colores desde la app

// Callback para recibir datos desde App Inventor
class MyCharacteristicCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      String value = pCharacteristic->getValue();

      if (value.length() > 0 && manualControl) {
        Serial.print("Comando recibido: ");
        Serial.println(value.c_str());

        if (value.equals("Rojo")) {
          rgbLedWrite(RGB_BUILTIN, BRIGHTNESS, 0, 0);
        } else if (value.equals("Verde")) {
          rgbLedWrite(RGB_BUILTIN, 0, BRIGHTNESS, 0);
        } else if (value.equals("Azul")) {
          rgbLedWrite(RGB_BUILTIN, 0, 0, BRIGHTNESS);
        }
      }
    }
};

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) { 
      deviceConnected = true; 
      connectionTime = millis(); // Guardamos el momento de la conexión
      manualControl = false;     // Bloqueamos control manual hasta que pase el tiempo azul
    };
    void onDisconnect(BLEServer* pServer) { 
      deviceConnected = false; 
    }
};

void flashLED(int r, int g, int b, int count) {
  for (int i = 0; i < count; i++) {
    rgbLedWrite(RGB_BUILTIN, r, g, b); delay(100);
    rgbLedWrite(RGB_BUILTIN, 0, 0, 0); delay(100);
  }
}

void setup() {
  Serial.begin(115200);
  BLEDevice::init(DEVICE_NAME);
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);
  
  // Creamos la característica
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHAR_UUID,
                                         BLECharacteristic::PROPERTY_READ | 
                                         BLECharacteristic::PROPERTY_WRITE | 
                                         BLECharacteristic::PROPERTY_NOTIFY
                                       );

  // IMPORTANTE: Asignamos los callbacks de ESCRITURA a la característica
  pCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
  pCharacteristic->addDescriptor(new BLE2902());
  
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  BLEDevice::startAdvertising();
  
  Serial.println("Esperando cliente...");
}

void loop() {
  // 1. Lógica al CONECTARSE
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
    Serial.println(">>> CONECTADO <<<");
    flashLED(0, 0, BRIGHTNESS, 3);
  }

  // 2. Lógica al DESCONECTARSE
  if (!deviceConnected && oldDeviceConnected) {
    Serial.println(">>> DESCONECTADO <<<");
    flashLED(BRIGHTNESS, 0, 0, 3);
    delay(500); 
    pServer->startAdvertising();
    oldDeviceConnected = deviceConnected;
    manualControl = false;
  }

  // 3. Gestión de colores en estado CONECTADO
  if (deviceConnected) {
    if (!manualControl) {
      // ESTO SOLO PASA DURANTE LOS PRIMEROS 3 SEGUNDOS
      rgbLedWrite(RGB_BUILTIN, 0, 0, BRIGHTNESS);
      
      if (millis() - connectionTime > 3000) {
        manualControl = true;
        delay(100);
        rgbLedWrite(RGB_BUILTIN, 0, 0, 0); // AHORA SÍ SE APAGARÁ
        Serial.println("Control manual activado y LED liberado");
      }
    }
    // Si manualControl es true, el loop no toca el LED. 
    // Queda libre para que el "onWrite" mande sus colores.
  } else {
    // Parpadeo verde de espera (se mantiene igual)
    if (millis() - lastBlink > 1000) {
      lastBlink = millis();
      blinkState = !blinkState;
      blinkState ? rgbLedWrite(RGB_BUILTIN, 0, BRIGHTNESS, 0) : rgbLedWrite(RGB_BUILTIN, 0, 0, 0);
    }
  }

  delay(100);
}
#endif