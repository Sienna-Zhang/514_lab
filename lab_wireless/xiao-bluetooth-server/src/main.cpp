#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <stdlib.h>

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
unsigned long previousMillis = 0;
const long interval = 1000;

// TODO: add new global variables for your sensor readings and processed data
// HC-SR04 pins
#define TRIG_PIN 4
#define ECHO_PIN 5

// raw & filtered distance
float rawDistance = 0.0;
float filteredDistance = 0.0;

// moving average DSP
#define FILTER_SIZE 5
float distanceBuffer[FILTER_SIZE];
int bufferIndex = 0;
bool bufferFilled = false;

// TODO: Change the UUID to your own (any specific one works, but make sure they're different from others'). You can generate one here: https://www.uuidgenerator.net/
#define SERVICE_UUID        "9f60ea96-04b9-47e6-9f15-2070e3a3ce5b"
#define CHARACTERISTIC_UUID "d866c44d-2845-4bce-b8b8-034dc50a8e91"

class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
    }
};

// TODO: add DSP algorithm functions here
float movingAverageFilter(float newValue) {
  distanceBuffer[bufferIndex] = newValue;
  bufferIndex = (bufferIndex + 1) % FILTER_SIZE;

  if (bufferIndex == 0) {
    bufferFilled = true;
  }

  int count = bufferFilled ? FILTER_SIZE : bufferIndex;
  float sum = 0.0;
  for (int i = 0; i < count; i++) {
    sum += distanceBuffer[i];
  }
  return sum / count;
}

void setup() {
    Serial.begin(115200);
    Serial.println("Starting BLE work!");

    // TODO: add codes for handling your sensor setup (pinMode, etc.)
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    digitalWrite(TRIG_PIN, LOW);

    // TODO: name your device to avoid conflictions
    BLEDevice::init("Pollyyao_Server");
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    BLEService *pService = pServer->createService(SERVICE_UUID);
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY
    );
    pCharacteristic->addDescriptor(new BLE2902());
    pCharacteristic->setValue("Hello World");
    pService->start();
    // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    Serial.println("Characteristic defined! Now you can read it in your phone!");
}

void loop() {
    // TODO: add codes for handling your sensor readings (analogRead, etc.)
    // HC-SR04 trigger
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    // read echo
    long duration = pulseIn(ECHO_PIN, HIGH, 30000);
    rawDistance = duration * 0.034 / 2.0;

    // TODO: use your defined DSP algorithm to process the readings
    filteredDistance = movingAverageFilter(rawDistance);
    Serial.print("Raw: ");
    Serial.print(rawDistance);
    Serial.print(" cm | Filtered: ");
    Serial.print(filteredDistance);
    Serial.println(" cm");

    if (deviceConnected) {
        // Send new readings to database
        // TODO: change the following code to send your own readings and processed data
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= interval) {  
             previousMillis = currentMillis;   
        if (filteredDistance < 30.0) {
            String msg = String(filteredDistance);
            pCharacteristic->setValue(msg.c_str());
            pCharacteristic->notify();
            Serial.println("Notify sent (filtered < 30 cm)");
        }
    }
}
    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500);  // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising();  // advertise again
        Serial.println("Start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
    delay(1000);
}
