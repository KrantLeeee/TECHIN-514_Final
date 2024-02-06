#include <Arduino.h>
#include <Wire.h>
#include <SparkFun_APDS9960.h>
#include <AccelStepper.h>

// Pin definitions for the stepper motor
#define motorPin1 D1  // IN1 on the ULN2003 driver
#define motorPin2 D0  // IN2 on the ULN2003 driver
#define motorPin3 D3  // IN3 on the ULN2003 driver
#define motorPin4 D2  // IN4 on the ULN2003 driver

#define APDS9960_INT D6 // Interrupt pin for APDS9960

// Global Variables
SparkFun_APDS9960 apds = SparkFun_APDS9960();
int isr_flag = 0;

// Initialize AccelStepper
AccelStepper stepper(AccelStepper::FULL4WIRE, motorPin1, motorPin3, motorPin2, motorPin4);

void interruptRoutine() {
  isr_flag = 1;
}

void handleGesture() {
    if ( apds.isGestureAvailable() ) {
    switch ( apds.readGesture() ) {
      case DIR_UP:
        Serial.println("UP");
        break;
      case DIR_DOWN:
        Serial.println("DOWN");
        break;
      case DIR_LEFT:
        Serial.println("LEFT");
        break;
      case DIR_RIGHT:
        Serial.println("RIGHT");
        break;
      case DIR_NEAR:
        Serial.println("NEAR");
        break;
      case DIR_FAR:
        Serial.println("FAR");
        break;
      default:
        Serial.println("NONE");
    }
  }
}

void setup() {
  // Initialize Serial port
  Serial.begin(115200);
  Serial.println();
  Serial.println(F("--------------------------------"));
  Serial.println(F("Gesture Controlled Stepper Motor"));
  Serial.println(F("--------------------------------"));

  // Initialize APDS-9960
  pinMode(APDS9960_INT, INPUT);
  attachInterrupt(digitalPinToInterrupt(APDS9960_INT), interruptRoutine, FALLING);
  if (apds.init()) {
    Serial.println(F("APDS-9960 initialization complete"));
  } else {
    Serial.println(F("Something went wrong during APDS-9960 init!"));
  }
  if (apds.enableGestureSensor(true)) {
    Serial.println(F("Gesture sensor is now running"));
  } else {
    Serial.println(F("Something went wrong during gesture sensor init!"));
  }

  // Stepper motor setup
  stepper.setMaxSpeed(1000);
  stepper.setAcceleration(500);
}

void loop() {
  if (isr_flag == 1) {
    handleGesture();
    isr_flag = 0;
  }
  stepper.run();
}

