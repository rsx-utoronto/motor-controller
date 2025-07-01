#include <Arduino.h>
#include <SimpleFOC.h>
#include "SimpleCAN.h"

#define LED_PIN PC6
#define BUTTON_PIN PC10

// Motor instance
BLDCMotor motor = BLDCMotor(7);
BLDCDriver6PWM driver = BLDCDriver6PWM(A_PHASE_UH, A_PHASE_UL, A_PHASE_VH, A_PHASE_VL, A_PHASE_WH, A_PHASE_WL);

float target = 1.0;
void serialLoop() {
  static String received_chars;

  while (Serial.available()) {
    char inChar = (char) Serial.read();
    received_chars += inChar;
    if (inChar == '\n') {
      target = received_chars.toFloat();
      Serial.print("Target = "); Serial.println(target);
      received_chars = "";
    }
  }
}

void setup() {
  
  // use monitoring with serial 
  Serial.begin(115200);
  // enable more verbose output for debugging
  // comment out if not needed
  // SimpleFOCDebug::enable(&Serial);
  
  // driver config
  // power supply voltage [V]
  driver.voltage_power_supply = 12;
  driver.pwm_frequency = 30000;
  driver.init();
  // link the motor and the driver
  motor.linkDriver(&driver);

  // set motion control loop to be used
  motor.controller = MotionControlType::velocity_openloop;

  motor.PID_velocity.P = 0.2;
  motor.PID_velocity.I = 20;

  // jerk control using voltage voltage ramp
  // default value is 300 volts per sec  ~ 0.3V per millisecond
  motor.PID_velocity.output_ramp = 1000;
 
  // velocity low pass filtering time constant
  motor.LPF_velocity.Tf = 0.01;

  // angle P controller
  motor.P_angle.P = 20;

  motor.voltage_limit = 12;
 
  motor.velocity_limit = 4;

  // comment out if not needed
  // motor.useMonitoring(Serial);
  
  // initialize motor
  motor.init();
  // align encoder and start FOC
  motor.initFOC();

  Serial.println(F("Motor ready."));
  Serial.println(F("Set the target angle using serial terminal:"));
  _delay(1000);
}

void loop() {
  // serialLoop();
  // main FOC algorithm function
  motor.loopFOC();

  // Motion control function
  motor.move(target);

  // function intended to be used with serial plotter to monitor motor variables
  // significantly slowing the execution down!!!!
  // motor.monitor();
}
