#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SparkFun_APDS9960.h>
#include <SwitecX25.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define STEPS 945 // Number of steps for 180 degrees
SwitecX25 motor(STEPS, D0, D1, D2, D3); // Pins D0 to D3 for the stepper

SparkFun_APDS9960 apds;
#define APDS9960_INT 6 // Interrupt pin D6

int displayMode = 1; // Start with Mode 1 (Temperature)


void updateStepper() {
  // Simulate humidity data with random values
  int humidity = random(0,101); // 0 to 100%
  int targetStep = map(humidity, 0, 100, 0, STEPS);
  motor.setPosition(targetStep);
  motor.update();
}

void updateDisplay() {
  display.clearDisplay();
   display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  
  if (displayMode == 1) {
    float temp = 27; // Simulate temperature
    display.print("Temp: ");
    display.print(temp);
    display.println(" C");
  } else if (displayMode == 2) {
    float pressure = 1000; // Simulate atmospheric pressure
    display.print("Pressure: ");
    display.print(pressure);
    display.println(" hPa");
  }
  
  display.display();
}

void showError(String error) {
  display.clearDisplay();
  display.setCursor(0,0);
  display.print(error);
  display.display();
  delay(5000); // Show error for 5 seconds
  updateDisplay(); // Go back to the current mode display
}

void handleGesture() {
  if (apds.isGestureAvailable()) {
    switch (apds.readGesture()) {
      case DIR_UP:
        Serial.println("UP");
        break;
      case DIR_DOWN:
        Serial.println("DOWN");
        break;
      case DIR_LEFT:
        if (displayMode == 2) {
          displayMode = 1;
          updateDisplay();
        } else {
          showError("Please try another direction~");
        }
        break;
      case DIR_RIGHT:
        if (displayMode == 1) {
          displayMode = 2;
          updateDisplay();
        } else {
          showError("Please try another direction~");
        }
        break;
      default:
        Serial.println("NONE");
    }
  }
}


volatile int isr_flag = 0;
void interruptRoutine() {
  isr_flag = 1;
}


void setup() {
  Serial.begin(115200);
  motor.zero();
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  
  pinMode(APDS9960_INT, INPUT);
  attachInterrupt(digitalPinToInterrupt(APDS9960_INT), interruptRoutine, FALLING);
  if (apds.init()) {
    apds.enableGestureSensor(true);
  } else {
    Serial.println("Failed to initialize APDS9960");
  }

  updateDisplay(); // Initial display update
}

void loop() {
  motor.update();
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 10000) { // Update every 10 seconds
    lastUpdate = millis();
    updateStepper(); // Update stepper with "humidity"
    updateDisplay(); // Update display with temperature or pressure
  }
  
  if (isr_flag == 1) {
    handleGesture();
    isr_flag = 0;
  }
}