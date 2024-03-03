//This is Client Display

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SwitecX25.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define STEPS (315*3) // Standard X25.168 range 315 degrees at 1/3 degree steps
SwitecX25 motor(STEPS, D0, D1, D2, D3); // Example GPIO pins, adjust as necessary

#include <Adafruit_APDS9960.h>
Adafruit_APDS9960 apds;

// 控制切换模式
#define MODE_1 1
#define MODE_2 2
int currentMode = MODE_1;


// The remote service and characteristic we wish to connect to.
static BLEUUID serviceUUID("bbea1724-2b74-49d5-b43b-8ab452184add");
static BLEUUID charUUID("7d1f2724-ca58-4b58-87c9-7d0a0bccac41");

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;

// Variables for handling notifications and motor position updates
static boolean newDataReceived = false;
static unsigned long lastUpdateTime = 0; // Last update time for throttle
const long updateInterval = 5000; // Time interval for updates (5 seconds)
static float temperature, humidity, pressure; // Variables for sensor data

// Variables for Calculation max and min
static float maxTemperature = -100; // 初始化为极低值以确保任何实际温度都会更高
static float minTemperature = 100;  // 初始化为极高值以确保任何实际温度都会更低




class MyClientCallback : public BLEClientCallbacks {
    void onConnect(BLEClient* pclient) {
        Serial.println("Connected to ---Krant Weather Station---");
    }

    void onDisconnect(BLEClient* pclient) {
        connected = false;
        Serial.println("Disconnected");
    }
};

static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
    Serial.print("Received Data: ");
    String message;
    for (int i = 0; i < length; i++) {
        message += (char)pData[i];
    }
    Serial.println(message);

    // Parse the received data
    sscanf(message.c_str(), "T:%fC H:%f%% P:%fhPa", &temperature, &humidity, &pressure);
        // 更新最高和最低温度
    if (temperature > maxTemperature) {
        maxTemperature = temperature;
    }
    if (temperature < minTemperature) {
        minTemperature = temperature;
    }
    newDataReceived = true; // Set flag to true to indicate new data has been received
}

bool connectToServer() {
    Serial.println("Forming a connection to the weather station");

    BLEClient* pClient = BLEDevice::createClient();
    Serial.println(" - Created client");

    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remote BLE Server.
    pClient->connect(myDevice);
    Serial.println(" - Connected to server");

    // Obtain a reference to the service and characteristic
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
        Serial.print("Failed to find our service UUID: ");
        Serial.println(serviceUUID.toString().c_str());
        return false;
    }
    Serial.println(" - Found our service");

    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr) {
        Serial.print("Failed to find our characteristic UUID: ");
        Serial.println(charUUID.toString().c_str());
        return false;
    }
    Serial.println(" - Found our characteristic");

    // Subscribe to the characteristic
    if(pRemoteCharacteristic->canNotify())
        pRemoteCharacteristic->registerForNotify(notifyCallback);

    connected = true;
    return true;
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        Serial.print("BLE Advertised Device found: ");
        Serial.println(advertisedDevice.toString().c_str());

        // Check for the specific service UUID
        if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {
            BLEDevice::getScan()->stop();
            myDevice = new BLEAdvertisedDevice(advertisedDevice);
            doConnect = true;
            doScan = false;
        }
    }
};

void setup() {
    Serial.begin(115200);
    Serial.println("Starting Arduino BLE Client application...");
    BLEDevice::init("");

    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;);
    }
    display.display();
    delay(2000); // Pause for 2 seconds
    display.clearDisplay();

    // 简单测试步进电机
    motor.setPosition(0); // 将电机归零
    delay(1000); // 等待1秒

    // 初始化APDS9960传感器
    if (!apds.begin()) {
        Serial.println("Failed to initialize APDS9960! Please check your wiring.");
    } else {
        Serial.println("APDS9960 initialized!");
        apds.enableProximity(true);
        apds.enableGesture(true);
    }

    // Start scanning for BLE devices
    BLEScan* pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setInterval(1349);
    pBLEScan->setWindow(449);
    pBLEScan->setActiveScan(true);
    pBLEScan->start(5, false);
}

void loop() {
    // 检测手势
    uint8_t gesture = apds.readGesture();
    if (gesture == APDS9960_LEFT) {
        currentMode = MODE_2; // 切换到模式2
    } else if (gesture == APDS9960_RIGHT) {
        currentMode = MODE_1; // 切换回模式1
    };

    // 根据当前模式更新OLED显示内容
    if (newDataReceived && millis() - lastUpdateTime >= 2000) {
        display.clearDisplay();
        if (currentMode == MODE_1) {
            // 显示当前温度和湿度（模式1），以及压力
            display.setTextSize(1); // 设置小字体
            display.setTextColor(SSD1306_WHITE); // 设置颜色为白色
            display.setCursor(0,0);
            display.print("Temp: ");
            display.setCursor(0,16);
            display.print("Hum: ");
            display.setCursor(0,32);
            display.print("Press: ");
            
            display.setTextSize(2); // 设置大字体
            display.setTextColor(SSD1306_WHITE); // 假设你有方式显示黄色，这里仍设为白色
            display.setCursor(48,0); // 调整数据显示的位置
            display.println(temperature, 2);
            display.setCursor(48,16);
            display.println(humidity, 2);
            display.setCursor(48,32);
            display.println(pressure, 2);
        } else if (currentMode == MODE_2) {
            // 显示最高温度和最低温度（模式2）
            display.setTextSize(1); // 设置小字体
            display.setTextColor(SSD1306_WHITE); // 设置颜色为白色
            display.setCursor(0,0);
            display.print("Max Temp: ");
            display.setCursor(0,16);
            display.print("Min Temp: ");
            
            display.setTextSize(2); // 设置大字体
            display.setTextColor(SSD1306_WHITE); // 假设你有方式显示黄色，这里仍设为白色
            display.setCursor(64,0); // 调整数据显示的位置
            display.println(maxTemperature, 2);
            display.setCursor(64,16);
            display.println(minTemperature, 2);
        }
        display.display();

        Serial.println("Inside update block");    
        // Convert humidity value to stepper motor position
        // Assuming 0-100% humidity maps to 0-180 degrees
        float targetAngle = (180.0 / 100.0) * humidity; // Convert humidity to angle
        float stepsPerDegree = STEPS / 315.0; // Calculate steps per degree
        int targetSteps = targetAngle * stepsPerDegree; // Convert target angle to steps
        int motorPosition = STEPS - targetSteps; // Calculate motor position from the end

        motor.setPosition(motorPosition); // Set motor position
        newDataReceived = false; // Reset data received flag
        lastUpdateTime = millis(); // Update last update time

        Serial.print("Setting motor to position: ");
        Serial.println(motorPosition);
    }


    motor.update(); // Update motor position

    // Handle BLE connection logic
    if (doConnect) {
        if (connectToServer()) {
            Serial.println("We are now connected to the BLE Server.");
        } else {
            Serial.println("We have failed to connect to the server; there is nothing more we will do.");
        }
        doConnect = false;
    }

    // If disconnected, initiate scanning again
    if (!connected && !doScan) {
        BLEDevice::getScan()->start(0); // continuous scanning
        doScan = true;
    }
}
