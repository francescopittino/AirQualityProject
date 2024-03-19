#include <Arduino.h>
#include "DHT.h"

//DHT Temperature and humidity sensor - library setup
#define DHT_PIN 4
#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);

void setup() {
  Serial.begin(115200);
  Serial.println("Setting up DHT11");
  dht.begin();
}

void loop() {
  delay(5000); //Update on 5 second mark
  float temp_c = dht.readTemperature(); //read temperature - celsius
  float temp_f = dht.readTemperature(true); // read temperature - fahrenheit
  float humidity = dht.readHumidity();
  //check results
  if(isnan(temp_c) || isnan(temp_f) || isnan(humidity)){
    Serial.println("Invalid measurement");
  }else{ //if everything's all right:
    //compute heat index:
    float heat_index = dht.computeHeatIndex(temp_c, humidity, false); // (temperature, humidity, isFahrenheit)
    //Output values
    Serial.println("Values from DHT11: ");
    Serial.print("Temperature (Celsius): ");
    Serial.println(temp_c);
    Serial.print("Temperature (Fahrenheit): ");
    Serial.println(temp_f);
    Serial.print("Humidity (%): ");
    Serial.println(humidity);
    Serial.print("Feels like: ");
    Serial.println(heat_index);
  }
}