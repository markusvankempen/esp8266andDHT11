// Example testing sketch for various DHT11 with esp8266
// mvk@ca.ibm.com - for SNP
// date: 13-Dec-2020 - Initial version
// 

#include "DHT.h"
#define VERSION "13122020-1411"

#define DHTPIN 4 // 4 = PIN D2   2 = PIN D4   // Digital pin connected to the DHT sensor

#define DHTTYPE DHT11   // DHT 11

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  Serial.println("DHT11 test!");
  Serial.println(VERSION);
  dht.begin();
}
float t=0;
float pt=0;
void loop() {
  // Wait a few seconds between measurements.
  delay(2000);


  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  pt =t;
  t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) ) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return; //restart
  }
  
   Serial.println("readTemperature in C = "+String(t));
   Serial.println("readHumidity in % = "+String(t));

}
