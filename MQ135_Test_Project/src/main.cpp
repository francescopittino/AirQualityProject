#include <Arduino.h>

#define pin 34 

void setup() {
  Serial.begin(115200);
  
  delay(1000);
}

void loop() {
  float res = analogRead(pin);
  Serial.print(res);
  Serial.println(" ppm");
  Serial.println("----------------------");
  delay(5000);
}