//This is Server Sensor

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERVICE_UUID        "bbea1724-2b74-49d5-b43b-8ab452184add"
#define CHARACTERISTIC_UUID "7d1f2724-ca58-4b58-87c9-7d0a0bccac41"

const int MA_SIZE = 10; // 移动平均的大小，例如取最近10个数据点的平均值
float temperatureHistory[MA_SIZE];
float humidityHistory[MA_SIZE];
float pressureHistory[MA_SIZE];
int historyIndex = 0; // 用于追踪当前应该更新哪个数组索引的变量

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
Adafruit_BME280 bme; // I2C

float updateHistoryAndCalculateAverage(float *history, float newValue) {
    // 更新历史数据
    history[historyIndex] = newValue;
    
    // 计算平均值
    float sum = 0;
    for (int i = 0; i < MA_SIZE; i++) {
        sum += history[i];
    }
    float average = sum / MA_SIZE;
    
    return average; // 返回计算出的平均值
}

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
        Serial.println("---Krant Weather Station---");
    };

    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
    }
};

void setup() {
    Serial.begin(115200);
    if (!bme.begin(0x76)) { // Auto-detect BME280 sensor
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        while (1);
    }

    BLEDevice::init("Krant Weather Station");
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

    pService->start();
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    Serial.println("Waiting for a client connection to notify...");
}

void loop() {
    if (deviceConnected) {
        float temperature = bme.readTemperature();
        float humidity = bme.readHumidity();
        float pressure = bme.readPressure() / 100.0F; // Convert from Pa to hPa

        // 更新历史数据并计算移动平均值
        float avgTemperature = updateHistoryAndCalculateAverage(temperatureHistory, temperature);
        float avgHumidity = updateHistoryAndCalculateAverage(humidityHistory, humidity);
        float avgPressure = updateHistoryAndCalculateAverage(pressureHistory, pressure);

        // 更新历史数据索引
        historyIndex = (historyIndex + 1) % MA_SIZE;

        // 使用平均值更新BLE特征值
        char data[80];
        snprintf(data, sizeof(data), "T:%.1fC H:%.1f%% P:%.0fhPa", avgTemperature, avgHumidity, avgPressure);
        pCharacteristic->setValue(data);
        pCharacteristic->notify();

        delay(1000); // 延迟以减少数据更新频率
    }

    if (!deviceConnected) {
        delay(500);
        pServer->startAdvertising();
    }
}
