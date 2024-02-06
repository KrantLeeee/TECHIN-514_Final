#include <Arduino.h>
#include <Wire.h>
#include <SparkFun_APDS9960.h>
#include <Stepper.h>

#define APDS9960_INT 6 // Interrupt pin for APDS9960
#define MOTOR_STEPS 200 // Number of steps per revolution, adjust as needed
#define MOTOR_PIN1 D1
#define MOTOR_PIN2 D2
#define MOTOR_PIN3 D3
#define MOTOR_PIN4 D0

SparkFun_APDS9960 apds;
Stepper myStepper(MOTOR_STEPS, MOTOR_PIN1, MOTOR_PIN3, MOTOR_PIN2, MOTOR_PIN4);
int isr_flag = 0;

void interruptRoutine() {
  isr_flag = 1;
}

void handleGesture() {
  if (apds.isGestureAvailable()) {
    switch (apds.readGesture()) {
      case DIR_LEFT:
        Serial.println("LEFT");
        myStepper.step(-MOTOR_STEPS / 4); // Turn stepper 90 degrees left
        break;
      case DIR_RIGHT:
        Serial.println("RIGHT");
        myStepper.step(MOTOR_STEPS / 4); // Turn stepper 90 degrees right
        break;
      // Add other gesture cases if needed
    }
  }
}


void setup() {
  Serial.begin(9600);
  attachInterrupt(APDS9960_INT, interruptRoutine, FALLING);

  if (apds.init() && apds.enableGestureSensor(true)) {
    Serial.println("Gesture sensor initialized");
  } else {
    Serial.println("Gesture sensor initialization failed!");
  }

  myStepper.setSpeed(60); // Set the speed of the stepper motor (RPM)
}

void loop() {
  if (isr_flag == 1) {
    handleGesture();
    isr_flag = 0;
  }
}

