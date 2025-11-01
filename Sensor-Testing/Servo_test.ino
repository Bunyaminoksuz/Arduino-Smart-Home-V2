#include <Servo.h>

#define KAPI_SERVO_PIN 2
#define BARIYER_SERVO_PIN 3
#define PERDE_SERVO_PIN 4

// Trafik lambası LEDleri
#define KIRMIZI_LED_PIN 24
#define SARI_LED_PIN 25
#define YESIL_LED_PIN 26

Servo kapiServo;
Servo bariyerServo;
Servo perdeServo;

void setup() {
  Serial.begin(9600);
  
  // Servo motorları başlat
  kapiServo.attach(KAPI_SERVO_PIN);
  bariyerServo.attach(BARIYER_SERVO_PIN);
  perdeServo.attach(PERDE_SERVO_PIN);
  
  // Trafik ışığı LEDleri
  pinMode(KIRMIZI_LED_PIN, OUTPUT);
  pinMode(SARI_LED_PIN, OUTPUT);
  pinMode(YESIL_LED_PIN, OUTPUT);
  
  Serial.println("Servo ve Bariyer Test Programi");
  Serial.println("1: Kapi Servo Test");
  Serial.println("2: Bariyer Servo ve Trafik Lambasi Test");
  Serial.println("3: Perde Servo Test");
}

void loop() {
  if (Serial.available() > 0) {
    char secim = Serial.read();
    
    if (secim == '1') {
      // Kapı servo testi
      Serial.println("Kapi aciliyor...");
      kapiServo.write(90);
      delay(2000);
      Serial.println("Kapi kapaniyor...");
      kapiServo.write(0);
    }
    else if (secim == '2') {
      // Bariyer servo ve trafik lambası testi
      // Kırmızı ışık - Bariyer kapalı
      digitalWrite(KIRMIZI_LED_PIN, HIGH);
      digitalWrite(SARI_LED_PIN, LOW);
      digitalWrite(YESIL_LED_PIN, LOW);
      bariyerServo.write(0);
      Serial.println("Bariyer kapali (Kirmizi isik yaniyor)");
      delay(2000);
      
      // Sarı ışık - Hazırlık
      digitalWrite(KIRMIZI_LED_PIN, LOW);
      digitalWrite(SARI_LED_PIN, HIGH);
      digitalWrite(YESIL_LED_PIN, LOW);
      Serial.println("Hazirlik (Sari isik yaniyor)");
      delay(2000);
      
      // Yeşil ışık - Bariyer açık
      digitalWrite(KIRMIZI_LED_PIN, LOW);
      digitalWrite(SARI_LED_PIN, LOW);
      digitalWrite(YESIL_LED_PIN, HIGH);
      bariyerServo.write(90);
      Serial.println("Bariyer acik (Yesil isik yaniyor)");
      delay(3000);
      
      // Tekrar kapat - döngü sonu
      digitalWrite(KIRMIZI_LED_PIN, HIGH);
      digitalWrite(SARI_LED_PIN, LOW);
      digitalWrite(YESIL_LED_PIN, LOW);
      bariyerServo.write(0);
      Serial.println("Bariyer kapali (Kirmizi isik yaniyor)");
    }
    else if (secim == '3') {
      // Perde servo testi
      Serial.println("Perde aciliyor...");
      perdeServo.write(90);
      delay(2000);
      Serial.println("Perde kapaniyor...");
      perdeServo.write(0);
    }
  }
}