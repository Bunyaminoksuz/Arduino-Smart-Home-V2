#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// DHT11 Sensör
#define DHT_SENSOR_PIN 31
#define DHT_TYPE DHT11

// MQ-2 Gaz Sensörü
#define GAS_SENSOR_PIN A0

// Yağmur Sensörü
#define YAGMUR_SENSOR_PIN A1

// Alarmlar için
#define BUZZER_PIN 10

// LCD Ekran
LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHT_SENSOR_PIN, DHT_TYPE);

void setup() {
  Serial.begin(9600);
  
  // LCD başlatma
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sensor Test");
  
  // DHT başlatma
  dht.begin();
  
  // Buzzer pin modu
  pinMode(BUZZER_PIN, OUTPUT);
  
  Serial.println("Entegre Sensor Test Programi");
}

void loop() {
  // DHT11 sensörü okuma
  float nem = dht.readHumidity();
  float sicaklik = dht.readTemperature();
  
  // Gaz sensörü okuma
  int gazDegeri = analogRead(GAS_SENSOR_PIN);
  
  // Yağmur sensörü okuma
  int yagmurDegeri = analogRead(YAGMUR_SENSOR_PIN);
  
  // Serial monitöre yazdır
  Serial.println("--- SENSORLER ---");
  
  if (!isnan(sicaklik) && !isnan(nem)) {
    Serial.print("Sicaklik: ");
    Serial.print(sicaklik);
    Serial.println(" C");
    
    Serial.print("Nem: %");
    Serial.println(nem);
  } else {
    Serial.println("DHT11 sensoru okunamadi!");
  }
  
  Serial.print("Gaz Seviyesi: ");
  Serial.println(gazDegeri);
  
  Serial.print("Yagmur Durumu: ");
  Serial.println(yagmurDegeri);
  
  // LCD'ye yazdır
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("S:");
  lcd.print(sicaklik);
  lcd.print("C");
  lcd.print(" N:%");
  lcd.print(nem);
  
  lcd.setCursor(0, 1);
  lcd.print("G:");
  lcd.print(gazDegeri);
  lcd.print(" Y:");
  lcd.print(yagmurDegeri);
  
  // Gaz alarmı
  if (gazDegeri > 400) {
    tone(BUZZER_PIN, 1000, 200);
    Serial.println("!!! GAZ ALARMI !!!");
  }
  
  delay(2000);
}