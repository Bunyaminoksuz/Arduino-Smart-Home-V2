#include <SPI.h>
#include <MFRC522.h>

#define RFID_SS_PIN 53
#define RFID_RST_PIN 49

MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  
  Serial.println("RFID Kart Okuyucu Test Programi");
  Serial.println("Lutfen bir kart yaklaştirin...");
}

void loop() {
  // Yeni bir kart okunduysa
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    // Kart UID'sini yazdır
    Serial.print("Kart UID: ");
    for (byte i = 0; i < rfid.uid.size; i++) {
      Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
      Serial.print(rfid.uid.uidByte[i], HEX);
    }
    Serial.println();
    
    // Kart UID'sini Hex formatında yazdır (Ana projenizde kullanmak için)
    Serial.print("Ana projede kullanmak icin: {0x");
    for (byte i = 0; i < rfid.uid.size; i++) {
      Serial.print(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
      Serial.print(rfid.uid.uidByte[i], HEX);
      if (i < rfid.uid.size - 1) {
        Serial.print(", 0x");
      }
    }
    Serial.println("}");
    
    // Kartı sıfırla
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    
    delay(1000);
  }
}