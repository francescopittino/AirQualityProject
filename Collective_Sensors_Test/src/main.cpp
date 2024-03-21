#include <Arduino.h>
#include "DHT.h"
#include <PMserial.h>
#include <Wire.h>    // I2C library
#include "ccs811.h"  // CCS811 library
#ifndef ESP32
#include <SoftwareSerial.h>
#endif
//DHT Temperature and humidity sensor - library setup
#define DHT_PIN 4
#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);

//MQ135 setup
#define MQ_PIN 34 

//PMS5003 Setup - using 16/17 as RX/TX
#if !defined(PMS_RX) && !defined(PMS_TX)
constexpr auto PMS_RX = 16;
constexpr auto PMS_TX = 17;
#endif

#ifndef ESP32
SerialPM pms(PMS5003, PMS_RX, PMS_TX); // PMSx003, RX, TX
// Alternative:
//SoftwareSerial SoftSerial1(PMS_RX, PMS_TX);
//SerialPM pms(PMS5003, SoftSerial1);
#else
SerialPM pms(PMS5003, PMS_RX, PMS_TX); // PMSx003, RX, TX
#endif

//CCS811 Setup
CCS811 ccs811(23); // nWAKE on 23

void DHT11Measurement(){
  //Gather measurements from DHT11
  //Read humidity and temperature(1 = fahrenheit, 0 or null = celsius)
  float temp_c = dht.readTemperature();
  float temp_f = dht.readTemperature(true);
  float humidity = dht.readHumidity();
  //Check measurements validity
  if(isnan(temp_c) || isnan(temp_f) || isnan(humidity)){
    Serial.println("Warning: DHT11: Invalid measurement, next measurement in t-10 seconds"); // Not valid
  }else{ //Measurement is valid
    //The "Feels Like" value:
    float heat_index = dht.computeHeatIndex(temp_c, humidity, false); //computeHeatIndex(temperature, humidity, isFahrenheit)
    //Output on Serial
    Serial.print("DHT11_Temperature_Celsius:");
    Serial.println(temp_c);
    Serial.print("DHT11_Temperature_Fahrenheit:");
    Serial.println(temp_f);
    Serial.print("DHT11_Humidity:");
    Serial.println(humidity);
    Serial.print("DHT11_Feels_Like:");
    Serial.println(heat_index);
  }
}

void MQ135Measurement(){
  //Read mixed measurement from pin
  float res = analogRead(MQ_PIN);
  //And print it on serial
  Serial.print("MQ135:");
  Serial.println(res);
}

void PMS5003Measurement(){
  pms.read();
  if(pms){ //Successfull read
    Serial.print("PM1.0:");
    Serial.println(pms.pm01);
    Serial.print("PM2.5:");
    Serial.println(pms.pm25);
    Serial.print("PM10:");
    Serial.println(pms.pm10);
    if (pms.has_number_concentration()){
      Serial.print("N0.3:");
      Serial.println(pms.n0p3);
      Serial.print("N0.5:");
      Serial.println(pms.n0p5);
      Serial.print("N1.0:");
      Serial.println(pms.n1p0);
      Serial.print("N2.5:");
      Serial.println(pms.n2p5);
      Serial.print("N5.0:");
      Serial.println(pms.n5p0);
      Serial.print("N10:" );
      Serial.println(pms.n10p0);
    }
  }else{ //Read not succesfull
    switch (pms.status){
      case pms.OK: //Should never come here, Included to compile with no warnings
        break;     
      case pms.ERROR_TIMEOUT:
        Serial.println(F(PMS_ERROR_TIMEOUT));
        break;
      case pms.ERROR_MSG_UNKNOWN:
        Serial.println(F(PMS_ERROR_MSG_UNKNOWN));
        break;
      case pms.ERROR_MSG_HEADER:
        Serial.println(F(PMS_ERROR_MSG_HEADER));
        break;
      case pms.ERROR_MSG_BODY:
        Serial.println(F(PMS_ERROR_MSG_BODY));
        break;
      case pms.ERROR_MSG_START:
        Serial.println(F(PMS_ERROR_MSG_START));
        break;
      case pms.ERROR_MSG_LENGTH:
        Serial.println(F(PMS_ERROR_MSG_LENGTH));
        break;
      case pms.ERROR_MSG_CKSUM:
        Serial.println(F(PMS_ERROR_MSG_CKSUM));
        break;
      case pms.ERROR_PMS_TYPE:
        Serial.println(F(PMS_ERROR_PMS_TYPE));
        break;
    }
  }
}

void CCS811Measurement(){
  uint16_t eco2, etvoc, errstat, raw;
  ccs811.read(&eco2,&etvoc,&errstat,&raw);
  // Print measurement results based on status
  if( errstat==CCS811_ERRSTAT_OK ) { 
    Serial.print("CCS811: ");
    Serial.print("eco2=");  Serial.print(eco2);     Serial.print(" ppm  ");
    Serial.print("etvoc="); Serial.print(etvoc);    Serial.print(" ppb  ");
    //Serial.print("raw6=");  Serial.print(raw/1024); Serial.print(" uA  "); 
    //Serial.print("raw10="); Serial.print(raw%1024); Serial.print(" ADC  ");
    //Serial.print("R="); Serial.print((1650*1000L/1023)*(raw%1024)/(raw/1024)); Serial.print(" ohm");
    Serial.println();
  } else if( errstat==CCS811_ERRSTAT_OK_NODATA ) {
    Serial.println("CCS811: waiting for (new) data");
  } else if( errstat & CCS811_ERRSTAT_I2CFAIL ) { 
    Serial.println("CCS811: I2C error");
  } else {
    Serial.print("CCS811: errstat="); Serial.print(errstat,HEX); 
    Serial.print("="); Serial.println( ccs811.errstat_str(errstat) ); 
  }
} 

void setup() {
  Serial.begin(115200);
  // Setup DHT11
  Serial.println("Setting up DHT11");
  try{
    dht.begin();
  }catch(const std::exception& e){
    Serial.println("ERROR: Could not setup DHT11. Check your wiring");
  } 
  delay(1000);
  //Setup PMS5003
  pms.init();
  //Setup CCS811
  // Enable I2C
  Wire.begin(); 

  // Enable CCS811
  ccs811.set_i2cdelay(50); // Needed for ESP8266 because it doesn't handle I2C clock stretch correctly
  bool ok= ccs811.begin();
  if( !ok ) Serial.println("setup: CCS811 begin FAILED");

  // Print CCS811 versions
  Serial.print("setup: hardware    version: "); Serial.println(ccs811.hardware_version(),HEX);
  Serial.print("setup: bootloader  version: "); Serial.println(ccs811.bootloader_version(),HEX);
  Serial.print("setup: application version: "); Serial.println(ccs811.application_version(),HEX);

  // Start measuring
  ok= ccs811.start(CCS811_MODE_1SEC);
  if( !ok ) Serial.println("setup: CCS811 start FAILED");
}

void loop() {
  DHT11Measurement(); //Temperature (in celsius and fahrenheit), humidity (%) and Heat index (Celsius)
  MQ135Measurement(); //Mixed measurement of the hazardous things mq135 can detect
  PMS5003Measurement(); //Particulate matter measurement
  CCS811Measurement(); // TVOC & eCO2 Measurement
  Serial.println("\n\n\n-----EOM-----\n\n\n");
  delay(10000); //Update every 10 seconds
}