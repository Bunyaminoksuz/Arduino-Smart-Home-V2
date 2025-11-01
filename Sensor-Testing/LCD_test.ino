#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// 0x27 veya 0x3F LCD adresiniz olabilir
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(9600);
  
  // LCD başlatma
  lcd.init();
  lcd.backlight();
  
  Serial.println("LCD Test Programi");
}

void loop() {
  // Test 1: Metin Yazma
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("LCD Test");
  lcd.setCursor(0, 1);
  lcd.print("Satir 2");
  delay(2000);
  
  // Test 2: Sayaç
  for (int i = 0; i <= 5; i++) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sayac: ");
    lcd.print(i);
    delay(1000);
  }
  
  // Test 3: Kayan Yazı
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Kayan Yazi");
  String mesaj = "Akilli Ev Sistemleri ";
  for (int pozisyon = 0; pozisyon < mesaj.length(); pozisyon++) {
    lcd.setCursor(0, 1);
    lcd.print(mesaj.substring(pozisyon, pozisyon + 16));
    delay(300);
  }
}