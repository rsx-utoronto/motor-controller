#include <Arduino.h>
#include "SimpleCAN.h"
#include <SimpleFOC.h>

#define LED_PIN PC6
#define BUTTON_PIN PC10

void setup() {
  Serial.begin(115200);
  while (!Serial); // Wait for USB to connect
  Serial.println("Hello World!");

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  digitalWrite(LED_PIN, LOW); // Turn off LED
}

void loop() {
  // Button pressed when pin is LOW (because of INPUT_PULLUP)
  if (digitalRead(BUTTON_PIN) == LOW) {
    Serial.println("Button Pressed!");
    digitalWrite(LED_PIN, HIGH); // Turn on LED
  } else {
    digitalWrite(LED_PIN, LOW);  // Turn off LED
  }
}
