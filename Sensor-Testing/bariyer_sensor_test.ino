#define BARIYER_SENSOR_PIN 41   // Sensör çıkışını bağladığınız pin
#define KIRMIZI_LED_PIN 24      // Kırmızı LED (Bariyer Kapalı)
#define YESIL_LED_PIN 26        // Yeşil LED (Bariyer Açık)
#define BARIYER_SERVO_PIN 3     // Bariyer servo motorunun bağlı olduğu pin

#include <Servo.h>
Servo bariyerServo;

void setup() {
  Serial.begin(9600);
  pinMode(BARIYER_SENSOR_PIN, INPUT);
  pinMode(KIRMIZI_LED_PIN, OUTPUT);
  pinMode(YESIL_LED_PIN, OUTPUT);
  
  bariyerServo.attach(BARIYER_SERVO_PIN);
  bariyerServo.write(0); // Başlangıçta bariyer kapalı
  
  digitalWrite(KIRMIZI_LED_PIN, HIGH); // Kırmızı ışık açık (Bariyer kapalı)
  digitalWrite(YESIL_LED_PIN, LOW);    // Yeşil ışık kapalı
  
  Serial.println("Kizilotesi Bariyer Sensoru Test Programi");
  Serial.println("Sensore bir nesne yaklaştirin veya sensor huzmesini kesin");
}

void loop() {
  int sensorDurum = digitalRead(BARIYER_SENSOR_PIN);
  
  // Sensörün aktif durumunu kontrol edin (LOW veya HIGH)
  // NOT: Sensör tipine göre bu değişebilir, test ederek doğru değeri bulun
  if (sensorDurum == LOW) {
    Serial.println("Bariyer Kesintisi Algilandi!");
    
    // Bariyer açık
    bariyerServo.write(90);
    digitalWrite(KIRMIZI_LED_PIN, LOW);
    digitalWrite(YESIL_LED_PIN, HIGH);
    
    delay(3000); // 3 saniye açık tut
    
    // Bariyer kapat
    bariyerServo.write(0);
    digitalWrite(KIRMIZI_LED_PIN, HIGH);
    digitalWrite(YESIL_LED_PIN, LOW);
    
    Serial.println("Bariyer kapandi.");
    delay(1000); // Tekrar tetiklemeyi önlemek için kısa bir bekleme
  }
  
  delay(100); // Küçük bir gecikme
}