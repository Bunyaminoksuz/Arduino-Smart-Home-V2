#include <DHT.h>

#define DHT_SENSOR_PIN 31
#define DHT_TYPE DHT11

DHT dht(DHT_SENSOR_PIN, DHT_TYPE);

void setup() {
  Serial.begin(9600);
  dht.begin();
  
  Serial.println("DHT11 Sicaklik ve Nem Sensoru Test Programi");
}

void loop() {
  delay(2000); // DHT11 sensörü 2 saniyede bir okumaya izin verir
  
  float nem = dht.readHumidity();
  float sicaklik = dht.readTemperature();
  
  if (isnan(nem) || isnan(sicaklik)) {
    Serial.println("DHT11 sensoru okunamadi!");
    return;
  }
  
  Serial.print("Nem: %");
  Serial.print(nem);
  Serial.print(" | Sicaklik: ");
  Serial.print(sicaklik);
  Serial.println(" C");
  
  // Hissedilen sıcaklık hesaplama
  float hissedilen = dht.computeHeatIndex(sicaklik, nem, false);
  Serial.print("Hissedilen Sicaklik: ");
  Serial.print(hissedilen);
  Serial.println(" C");
  
  Serial.println("---------------------");
}