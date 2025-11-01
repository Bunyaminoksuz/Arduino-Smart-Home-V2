/**
 * Akıllı Ev otomasyon Sistemi:
 * - RFID ile kapı kontrolü
 * - IR kumanda ile kontrol (perde, lamba, priz)
 * - Gaz, yangın, deprem algılama ve uyarı
 * - Hareket algılama ve otomatik ışık kontrolü
 * - Sıcaklık, nem ve hava durumu izleme
 * - Trafik ışıklı bariyer sistemi
 * - 4 kanallı röle kontrolü (lamba, gaz vanası, priz, yangın söndürme)
 */

#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <IRremote.h>

// I2C LCD Ekran - 0x27 tipik I2C adresidi, Eğer Olmaz İse 0x3F ile Değiştirin.
LiquidCrystal_I2C lcd(0x27, 16, 2);

// IR Kumanda
#define IR_RECEIVE_PIN 12
IRrecv irrecv(IR_RECEIVE_PIN);
decode_results results;

// IR Kumanda Tuş Kodları - kullandığınız kumandaya göre değiştir
#define IR_PERDE_AC      0xBA45FF00 // 1 tuşu
#define IR_PERDE_KAPAT   0xB946FF00 // 2 tuşu
#define IR_LAMBA_TOGGLE  0xB847FF00 // 3 tuşu
#define IR_PRIZ_TOGGLE   0xBB44FF00 // 4 tuşu
#define IR_BARIYER_AC    0xBF40FF00 // 5 tuşu (örnek kod)
#define IR_BARIYER_KAPAT 0xBC43FF00 // 6 tuşu (örnek kod)

// Pin Tanımlamaları
#define RFID_SS_PIN 53
#define RFID_RST_PIN 49

#define KAPI_SERVO_PIN 2
#define BARIYER_SERVO_PIN 3
#define PERDE_SERVO_PIN 4

// Röle Pin Tanımlamaları
#define ROLE_LAMBA_PIN 37           // Röle 1: Lamba
#define ROLE_GAZ_VANASI_PIN 38      // Röle 2: Gaz Vanası
#define ROLE_PRIZ_PIN 39            // Röle 3: Priz
#define ROLE_YANGIN_SONDURME_PIN 40 // Röle 4: Yangın Söndürme

// Trafik lambası LEDleri
#define KIRMIZI_LED_PIN 24
#define SARI_LED_PIN 25
#define YESIL_LED_PIN 26

// Alarm LEDleri
#define DEPREM_LED_PIN 27
#define GAZ_YANGIN_LED_PIN 28
#define HAREKET_LED_PIN 29
#define MESAFE_ALARM_LED_PIN 30

// Sensörler
#define DHT_SENSOR_PIN 31
#define DHT_TYPE DHT11
#define ALEV_SENSOR_PIN 32
#define PIR_SENSOR_PIN 33
#define TILT_SENSOR_PIN 34
#define GAS_SENSOR_PIN A0
#define YAGMUR_SENSOR_PIN A1

// HC-SR04 Ultrasonik Mesafe Sensörü
#define MESAFE_TRIG_PIN 35
#define MESAFE_ECHO_PIN 36

// Buzzer
#define BUZZER_PIN 10

// Kızılötesi Bariyer Sensörü
#define BARIYER_SENSOR_PIN 7  

// Sensor Nesneleri
MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);
Servo kapiServo;
Servo bariyerServo;
Servo perdeServo;
DHT dht(DHT_SENSOR_PIN, DHT_TYPE);

// Sabitler
const int GAS_ESIK = 400;    // Gaz sensörü eşik değeri
const int YAGMUR_ESIK_SIDDETLI = 300; // Şiddetli fırtına için eşik
const int YAGMUR_ESIK_ORTA = 500;     // Fırtına için eşik
const int YAGMUR_ESIK_AZ = 700;       // Sağanak yağmur için eşik
const int MESAFE_MIN = 10;  // Minimum algılama mesafesi cm
const int MESAFE_MAX = 15;  // Maksimum algılama mesafesi cm
const int BARIYER_GECIS_SURESI = 5000; // Araç geçtikten sonra beklenecek süre (5 saniye)

// İzin verilen RFID kartları (örnek UID'ler)
byte izinliKartlar[][4] = {
  {0x12, 0x34, 0x56, 0x78}, // Kart 1 - Kendi RFID kartınızın UID'si ile değiştirin
  {0xAA, 0xBB, 0xCC, 0xDD}  // Kart 2 - Kendi RFID kartınızın UID'si ile değiştirin
};
const int KART_SAYISI = 2;

// Değişkenler
bool kapiAcik = false;
bool perdeAcik = false;
bool bariyerAcik = false;
bool bariyerHareket = false;
bool depremDurumu = false;
bool gazKacagiDurumu = false;
bool yanginDurumu = false;
bool hareketAlgilandi = false;
bool mesafeAlarmDurumu = false;
bool bariyerDeSensorTetiklendi = false;
bool aracGecisiVarMi = false;

// Röle durumları
bool roleLambaDurum = false;       // Başlangıçta kapalı
bool roleGazVanasiDurum = true;    // Başlangıçta açık (güvenlik için)
bool rolePrizDurum = false;        // Başlangıçta kapalı
bool roleYanginSondurmeDurum = false; // Başlangıçta kapalı

// Zamanlama değişkenleri
unsigned long sonBuzzerZamani = 0;
unsigned long sonLcdGuncellemeZamani = 0;
unsigned long sonIrOkumaZamani = 0;
unsigned long bariyerHareketBaslangic = 0;
unsigned long aracGecisZamani = 0;
unsigned long simdikiZaman = 0;

void setup() {
  Serial.begin(9600);
  
  // LCD başlatma
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Akilli Ev Sistemi");
  lcd.setCursor(0, 1);
  lcd.print("Baslatiliyor...");

  // IR kumanda başlatma
  irrecv.enableIRIn();
  
  // RFID başlatma
  SPI.begin();
  rfid.PCD_Init();
  
  // Servo başlatma
  kapiServo.attach(KAPI_SERVO_PIN);
  bariyerServo.attach(BARIYER_SERVO_PIN);
  perdeServo.attach(PERDE_SERVO_PIN);
  
  kapiServo.write(0); // Kapı kapalı
  bariyerServo.write(0); // Bariyer kapalı
  perdeServo.write(0); // Perde kapalı
  
  // Röle pinleri
  pinMode(ROLE_LAMBA_PIN, OUTPUT);
  pinMode(ROLE_GAZ_VANASI_PIN, OUTPUT);
  pinMode(ROLE_PRIZ_PIN, OUTPUT);
  pinMode(ROLE_YANGIN_SONDURME_PIN, OUTPUT);
  
  // Başlangıçta röleleri ayarla (LOW = Açık, HIGH = Kapalı [Normalde kapalı röle modülleri için])
  digitalWrite(ROLE_LAMBA_PIN, roleLambaDurum ? LOW : HIGH);          // Lamba kapalı
  digitalWrite(ROLE_GAZ_VANASI_PIN, roleGazVanasiDurum ? LOW : HIGH); // Gaz vanası açık
  digitalWrite(ROLE_PRIZ_PIN, rolePrizDurum ? LOW : HIGH);            // Priz kapalı
  digitalWrite(ROLE_YANGIN_SONDURME_PIN, roleYanginSondurmeDurum ? LOW : HIGH); // Yangın söndürme kapalı

  // LEDler
  pinMode(KIRMIZI_LED_PIN, OUTPUT);
  pinMode(SARI_LED_PIN, OUTPUT);
  pinMode(YESIL_LED_PIN, OUTPUT);
  pinMode(DEPREM_LED_PIN, OUTPUT);
  pinMode(GAZ_YANGIN_LED_PIN, OUTPUT);
  pinMode(HAREKET_LED_PIN, OUTPUT);
  pinMode(MESAFE_ALARM_LED_PIN, OUTPUT);
  
  // Başlangıçta LEDleri kapat
  digitalWrite(KIRMIZI_LED_PIN, LOW);
  digitalWrite(SARI_LED_PIN, LOW);
  digitalWrite(YESIL_LED_PIN, LOW);
  digitalWrite(DEPREM_LED_PIN, LOW);
  digitalWrite(GAZ_YANGIN_LED_PIN, LOW);
  digitalWrite(HAREKET_LED_PIN, LOW);
  digitalWrite(MESAFE_ALARM_LED_PIN, LOW);
  
  // Sensörler
  pinMode(ALEV_SENSOR_PIN, INPUT);
  pinMode(PIR_SENSOR_PIN, INPUT);
  pinMode(TILT_SENSOR_PIN, INPUT);
  // Analog pinler için pinMode gerekli değil (GAS_SENSOR_PIN, YAGMUR_SENSOR_PIN)
  
  // Kızılötesi bariyer sensörü
  pinMode(BARIYER_SENSOR_PIN, INPUT);
  
  // DHT sensörü başlatma
  dht.begin();
  
  // Mesafe sensörü
  pinMode(MESAFE_TRIG_PIN, OUTPUT);
  pinMode(MESAFE_ECHO_PIN, INPUT);
  
  // Buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Bariyer işaretini güncelle (başlangıçta kırmızı)
  bariyerIsaretiniGuncelle();
  
  delay(2000); // Başlangıç mesajını göstermek için biraz bekle
  lcd.clear();
  Serial.println("Akilli Ev Sistemi baslatildi!");
}

void loop() {
  simdikiZaman = millis();
  
  // RFID ile kapı kontrolü
  rfidKontrol();
  
  // IR kumanda kontrol
  kumandayiOku();
  
  // Bariyer sensörü kontrolü
  bariyerSensorKontrol();
  
  // Bariyer kontrolü
  bariyerKontrol();
  
  // Tilt sensörü ile deprem algılama
  depremKontrol();
  
  // Hareket algılama
  hareketKontrol();
  
  // Yangın ve gaz algılama
  yanginGazKontrol();
  
  // Hava durumu kontrolü
  havaDurumuKontrol();
  
  // Mesafe sensörü kontrolü
  mesafeKontrol();
  
  // LCD ekranı periyodik olarak güncelle
  ekraniGuncelle();
  
  // Buzzer kontrolü
  buzzerKontrol();
}

// Kızılötesi bariyer sensörü kontrolü -
void bariyerSensorKontrol() {
  bool sensorDurum = digitalRead(BARIYER_SENSOR_PIN);
  
  // Sensör tetiklendiğinde (LOW = ışın kesildi, araba var)
  if (sensorDurum == LOW && !bariyerDeSensorTetiklendi) {
    bariyerDeSensorTetiklendi = true;
    aracGecisiVarMi = true;
    
    Serial.println("Bariyer: Arac algilandi!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Bariyer Sensoru");
    lcd.setCursor(0, 1);
    lcd.print("Arac Algilandi!");
    
    // Bariyeri aç
    if (!bariyerAcik) {
      bariyerAc();
    }
    
    aracGecisZamani = simdikiZaman; // Araç geçiş zamanını kaydet
  }
  // Sensör normale döndüğünde (HIGH = ışın geçiyor, araba yok)
  else if (sensorDurum == HIGH && bariyerDeSensorTetiklendi) {
    bariyerDeSensorTetiklendi = false;
    
    // Araç hala geçiyor olarak kabul et, otomatik kapanma için zamanlayıcı işlevi
    // aracGecisiVarMi değişkeni hala true kalacak
    
    Serial.println("Bariyer: Arac gecisi tamamlandi");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Bariyer");
    lcd.setCursor(0, 1);
    lcd.print("Arac gecti!");
    
    aracGecisZamani = simdikiZaman; // Araç geçiş tamamlanma zamanını güncelle
  }
  
  // Araç geçtikten 5 saniye sonra bariyeri kapat
  if (aracGecisiVarMi && !bariyerDeSensorTetiklendi && (simdikiZaman - aracGecisZamani > BARIYER_GECIS_SURESI)) {
    Serial.println("Bariyer: 5 saniye doldu, bariyer kapaniyor");
    aracGecisiVarMi = false;
    
    // Bariyeri kapat
    if (bariyerAcik) {
      bariyerKapat();
    }
  }
}

// Bariyer açma işlevi 
void bariyerAc() {
  bariyerAcik = true;
  bariyerHareket = true;
  bariyerHareketBaslangic = simdikiZaman;
  
  // Sarı ışığı yak (hareket ediyor)
  digitalWrite(KIRMIZI_LED_PIN, LOW);
  digitalWrite(SARI_LED_PIN, HIGH); 
  digitalWrite(YESIL_LED_PIN, LOW);
  
  // Bariyeri aç
  bariyerServo.write(90); // Bariyeri aç (90 derece)
  
  Serial.println("Bariyer aciliyor...");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Bariyer");
  lcd.setCursor(0, 1);
  lcd.print("Aciliyor...");
}

// Bariyer kapatma işlevi -
void bariyerKapat() {
  bariyerAcik = false;
  bariyerHareket = true;
  bariyerHareketBaslangic = simdikiZaman;
  
  // Sarı ışığı yak (hareket ediyor)
  digitalWrite(KIRMIZI_LED_PIN, LOW);
  digitalWrite(SARI_LED_PIN, HIGH);
  digitalWrite(YESIL_LED_PIN, LOW);
  
  // Bariyeri kapat
  bariyerServo.write(0); // Bariyeri kapat (0 derece)
  
  Serial.println("Bariyer kapaniyor...");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Bariyer");
  lcd.setCursor(0, 1);
  lcd.print("Kapaniyor...");
}

// RFID Kart kontrolü
void rfidKontrol() {
  // Yeni bir kart okunduysa
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    bool kartTanindi = false;
    
    // Okunan kartın UID'sini kontrol et
    for (int i = 0; i < KART_SAYISI; i++) {
      if (karsilastirUID(rfid.uid.uidByte, izinliKartlar[i])) {
        kartTanindi = true;
        break;
      }
    }
    
    // Kart tanındıysa kapıyı aç/kapat
    if (kartTanindi) {
      kapiAcik = !kapiAcik;
      
      if (kapiAcik) {
        Serial.println("Kapi aciliyor...");
        kapiServo.write(90); // Kapıyı aç
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Kapi aciliyor...");
      } else {
        Serial.println("Kapi kapaniyor...");
        kapiServo.write(0); // Kapıyı kapat
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Kapi kapaniyor...");
      }
    } else {
      Serial.println("Yetkisiz kart!");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Yetkisiz kart!");
      // Yetkisiz kart için uyarı sesi
      tone(BUZZER_PIN, 1000, 500);
    }
    
    // RFID okuyucuyu sıfırla
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }
}

// UID karşılaştırma fonksiyonu
bool karsilastirUID(byte *uid1, byte *uid2) {
  for (int i = 0; i < 4; i++) {
    if (uid1[i] != uid2[i]) {
      return false;
    }
  }
  return true;
}

// IR Kumanda okuma - GÜNCELLENDİ
void kumandayiOku() {
  // IR kumandayı çok sık okuma
  if (simdikiZaman - sonIrOkumaZamani < 100) {
    return;
  }
  sonIrOkumaZamani = simdikiZaman;
  
  if (irrecv.decode(&results)) {
    switch(results.value) {
      case IR_PERDE_AC:
        perdeKontrol(true);
        break;
      case IR_PERDE_KAPAT:
        perdeKontrol(false);
        break;
      case IR_LAMBA_TOGGLE:
        roleKontrol(1, !roleLambaDurum); // Lamba durumunu tersine çevir
        break;
      case IR_PRIZ_TOGGLE:
        roleKontrol(3, !rolePrizDurum); // Priz durumunu tersine çevir
        break;
      case IR_BARIYER_AC:  // YENİ EKLENEN
        if (!bariyerAcik) bariyerAc();
        break;
      case IR_BARIYER_KAPAT:  // YENİ EKLENEN
        if (bariyerAcik) bariyerKapat();
        break;
    }
    irrecv.resume(); // Sonraki değer için hazırlan
  }
}

// Perde kontrolü fonksiyonu
void perdeKontrol(bool ac) {
  if (ac && !perdeAcik) {
    perdeAcik = true;
    Serial.println("Perde aciliyor...");
    perdeServo.write(90); // Perdeyi aç - servo 90 derece
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Perde aciliyor...");
  } else if (!ac && perdeAcik) {
    perdeAcik = false;
    Serial.println("Perde kapaniyor...");
    perdeServo.write(0); // Perdeyi kapat - servo 0 derece
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Perde kapaniyor...");
  }
}

// Bariyer kontrolü - GÜNCELLENDİ
void bariyerKontrol() {
  // Bariyer hareketi kontrol
  if (bariyerHareket) {
    if (simdikiZaman - bariyerHareketBaslangic > 1000) { // 1 saniye sonra hareket biter
      bariyerHareket = false;
      bariyerIsaretiniGuncelle(); // Hareket bitince ışığı güncelle
    }
  }
}

// Bariyer işaretini güncelle (kırmızı, sarı, yeşil) - GÜNCELLENDİ
void bariyerIsaretiniGuncelle() {
  if (bariyerHareket) {
    // Bariyer hareket ediyor - Sarı ışık
    digitalWrite(KIRMIZI_LED_PIN, LOW);
    digitalWrite(SARI_LED_PIN, HIGH);
    digitalWrite(YESIL_LED_PIN, LOW);
  } else if (bariyerAcik) {
    // Bariyer açık - Yeşil ışık
    digitalWrite(KIRMIZI_LED_PIN, LOW);
    digitalWrite(SARI_LED_PIN, LOW);
    digitalWrite(YESIL_LED_PIN, HIGH);
  } else {
    // Bariyer kapalı - Kırmızı ışık
    digitalWrite(KIRMIZI_LED_PIN, HIGH);
    digitalWrite(SARI_LED_PIN, LOW);
    digitalWrite(YESIL_LED_PIN, LOW);
  }
}

// Bariyer aç/kapat fonksiyonu (eski) - SİLMİYORUZ, GEREKLİ OLABİLİR
void bariyerDurumunuDegistir() {
  if (!bariyerAcik) {
    bariyerAc();
  } else {
    bariyerKapat();
  }
}

// Deprem kontrolü
void depremKontrol() {
  int tiltDurum = digitalRead(TILT_SENSOR_PIN);
  
  if (tiltDurum == HIGH && !depremDurumu) { // Tilt algılandı, deprem olabilir
    depremDurumu = true;
    Serial.println("!!! DEPREM OLUYOR !!!");
    digitalWrite(DEPREM_LED_PIN, HIGH);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("!!! DEPREM OLUYOR !!!");
    lcd.setCursor(0, 1);
    lcd.print("TEHLIKE!");
  } else if (tiltDurum == LOW && depremDurumu) {
    depremDurumu = false;
    digitalWrite(DEPREM_LED_PIN, LOW);
  }
}

// Hareket kontrolü
void hareketKontrol() {
  int hareketDurum = digitalRead(PIR_SENSOR_PIN);
  
  if (hareketDurum == HIGH && !hareketAlgilandi) {
    hareketAlgilandi = true;
    Serial.println("Iceride hareket algilandi!");
    digitalWrite(HAREKET_LED_PIN, HIGH);
    
    // Hareket algılandığında otomatik olarak lambayı aç
    roleKontrol(1, true); // Lambayı aç
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Isiklar acildi");
    lcd.setCursor(0, 1);
    lcd.print("Hareket var!");
  } else if (hareketDurum == LOW && hareketAlgilandi) {
    hareketAlgilandi = false;
    digitalWrite(HAREKET_LED_PIN, LOW);
    
    // Burada istiyorsanız lambayı otomatik kapatabilirsiniz
    // roleKontrol(1, false); // Lambayı kapat
  }
}

// Yangın ve gaz kontrolü
void yanginGazKontrol() {
  int alevDurum = digitalRead(ALEV_SENSOR_PIN);
  int gazDurum = analogRead(GAS_SENSOR_PIN);
  
  bool oncekiGazDurumu = gazKacagiDurumu;
  bool oncekiYanginDurumu = yanginDurumu;
  
  // Gaz kaçağı kontrolü
  if (gazDurum > GAS_ESIK) {
    gazKacagiDurumu = true;
  } else {
    gazKacagiDurumu = false;
  }
  
  // Alev/yangın kontrolü
  if (alevDurum == LOW) { // Çoğu alev sensörü LOW olduğunda alev algılar
    yanginDurumu = true;
  } else {
    yanginDurumu = false;
  }
  
  // Durum değişti mi kontrol et
  if (gazKacagiDurumu != oncekiGazDurumu || yanginDurumu != oncekiYanginDurumu) {
    if (gazKacagiDurumu && yanginDurumu) {
      Serial.println("!!! YANGIN VE GAZ KACAGI ALARMI !!!");
      digitalWrite(GAZ_YANGIN_LED_PIN, HIGH);
      
      // Gaz vanasını kapat
      roleKontrol(2, false);
      
      // Yangın söndürme sistemini aktifleştir
      roleKontrol(4, true);
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("!!! TEHLIKE !!!");
      lcd.setCursor(0, 1);
      lcd.print("YANGIN & GAZ KACAGI VAR");
    } else if (gazKacagiDurumu) {
      Serial.println("!!! GAZ KACAGI ALARMI !!!");
      Serial.println("Gaz Degeri: " + String(gazDurum));
      digitalWrite(GAZ_YANGIN_LED_PIN, HIGH);
      
      // Gaz vanasını kapat
      roleKontrol(2, false);
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("!!! GAZ KACAGI VAR !!!");
      lcd.setCursor(0, 1);
      lcd.print("Deger: " + String(gazDurum));
    } else if (yanginDurumu) {
      Serial.println("!!! YANGIN ALARMI !!!");
      digitalWrite(GAZ_YANGIN_LED_PIN, HIGH);
      
      // Gaz vanasını kapat
      roleKontrol(2, false);
      
      // Yangın söndürme sistemini aktifleştir
      roleKontrol(4, true);
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("!!! YANGIN VAR !!!");
      lcd.setCursor(0, 1);
      lcd.print("TEHLIKE!");
    } else {
      digitalWrite(GAZ_YANGIN_LED_PIN, LOW);
      
      // Gaz kaçağı ve yangın durumu düzeldiyse
      if (roleYanginSondurmeDurum) {
        // Yangın söndürme sistemini kapat
        roleKontrol(4, false);
      }
      
      // Güvenlik açısından gaz vanasını otomatik açmamalıyız
      // Bu manuel olarak yapılmalı
      // roleKontrol(2, true);
    }
  }
}

// Hava durumu kontrolü
void havaDurumuKontrol() {
  float sicaklik = dht.readTemperature();
  float nem = dht.readHumidity();
  int yagmurDurum = analogRead(YAGMUR_SENSOR_PIN);
  
  if (!isnan(sicaklik) && !isnan(nem)) {
    // Periyodik güncelleme için main loop'taki ekraniGuncelle() fonksiyonu kullanılacak
    
    // Hava karlı mı kontrolü
    if (sicaklik < 0 && yagmurDurum < YAGMUR_ESIK_AZ) {
      Serial.println("Hava Durumu: Karli, Sicaklik: " + String(sicaklik) + "C, Nem: " + String(nem) + "%");
    }
    // Yağmur durumu sınıflandırma
    else if (yagmurDurum < YAGMUR_ESIK_SIDDETLI) {
      Serial.println("Hava Durumu: Siddetli Firtina, Sicaklik: " + String(sicaklik) + "C, Nem: " + String(nem) + "%");
    }
    else if (yagmurDurum < YAGMUR_ESIK_ORTA) {
      Serial.println("Hava Durumu: Firtina, Sicaklik: " + String(sicaklik) + "C, Nem: " + String(nem) + "%");
    }
    else if (yagmurDurum < YAGMUR_ESIK_AZ) {
      Serial.println("Hava Durumu: Saganak Yagmur, Sicaklik: " + String(sicaklik) + "C, Nem: " + String(nem) + "%");
    }
  }
}

// Mesafe sensörü ile izleme
void mesafeKontrol() {
  // HC-SR04 ile mesafe ölçümü
  digitalWrite(MESAFE_TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(MESAFE_TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(MESAFE_TRIG_PIN, LOW);
  
  long sure = pulseIn(MESAFE_ECHO_PIN, HIGH);
  int mesafe = sure * 0.034 / 2; // cm cinsinden mesafe
  
  if (mesafe >= MESAFE_MIN && mesafe <= MESAFE_MAX) {
    if (!mesafeAlarmDurumu) {
      mesafeAlarmDurumu = true;
      Serial.println("!!! DIKKAT !!! 10-15cm arasinda hareket algilandi!");
      Serial.println("Polis araniyor...");
      digitalWrite(MESAFE_ALARM_LED_PIN, HIGH);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("! DIKKAT !");
      lcd.setCursor(0, 1);
      lcd.print("Polis araniyor!");
    }
  } else if (mesafeAlarmDurumu) {
    mesafeAlarmDurumu = false;
    digitalWrite(MESAFE_ALARM_LED_PIN, LOW);
  }
}

// Röle kontrol fonksiyonu
void roleKontrol(int roleNo, bool ac) {
  switch(roleNo) {
    case 1: // Lamba
      roleLambaDurum = ac;
      digitalWrite(ROLE_LAMBA_PIN, ac ? LOW : HIGH); // LOW = Açık, HIGH = Kapalı
      
      Serial.print("Lamba ");
      Serial.println(ac ? "Acildi" : "Kapatildi");
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Lamba");
      lcd.setCursor(0, 1);
      lcd.print(ac ? "Acildi" : "Kapatildi");
      break;
      
    case 2: // Gaz Vanası
      roleGazVanasiDurum = ac;
      digitalWrite(ROLE_GAZ_VANASI_PIN, ac ? LOW : HIGH);
      
      Serial.print("Gaz Vanasi ");
      Serial.println(ac ? "Acildi" : "Kapatildi");
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Gaz Vanasi");
      lcd.setCursor(0, 1);
      lcd.print(ac ? "Acildi" : "Kapatildi");
      break;
      
    case 3: // Priz
      rolePrizDurum = ac;
      digitalWrite(ROLE_PRIZ_PIN, ac ? LOW : HIGH);
      
      Serial.print("Priz ");
      Serial.println(ac ? "Acildi" : "Kapatildi");
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Priz");
      lcd.setCursor(0, 1);
      lcd.print(ac ? "Acildi" : "Kapatildi");
      break;
      
    case 4: // Yangın Söndürme
      roleYanginSondurmeDurum = ac;
      digitalWrite(ROLE_YANGIN_SONDURME_PIN, ac ? LOW : HIGH);
      
      Serial.print("Yangin Sondurme ");
      Serial.println(ac ? "Aktif" : "Pasif");
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Yangin Sondurme");
      lcd.setCursor(0, 1);
      lcd.print(ac ? "Aktif" : "Pasif");
      break;
  }
  
  delay(1000); // Mesajı 1 saniye göster
}

// LCD ekranını periyodik olarak güncelle
void ekraniGuncelle() {
  // Belirli aralıklarla ekranı güncelle (her 5 saniyede bir)
  if (simdikiZaman - sonLcdGuncellemeZamani > 5000) {
    sonLcdGuncellemeZamani = simdikiZaman;
    
    // Eğer bir alarm durumu yoksa, normal durum bilgilerini göster
    if (!depremDurumu && !gazKacagiDurumu && !yanginDurumu && !mesafeAlarmDurumu) {
      float sicaklik = dht.readTemperature();
      float nem = dht.readHumidity();
      int yagmurDurum = analogRead(YAGMUR_SENSOR_PIN);
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Sicaklik:" + String(sicaklik) + "C");
      lcd.setCursor(0, 1);
      lcd.print("Nem: %" + String(nem));
      
      String havaDurumu = "";
      
      // Hava karlı mı kontrolü
      if (sicaklik < 0 && yagmurDurum < YAGMUR_ESIK_AZ) {
        havaDurumu = "Karli";
      }
      // Yağmur durumu sınıflandırma
      else if (yagmurDurum < YAGMUR_ESIK_SIDDETLI) {
        havaDurumu = "Siddetli Firtina";
      }
      else if (yagmurDurum < YAGMUR_ESIK_ORTA) {
        havaDurumu = "Firtina";
      }
      else if (yagmurDurum < YAGMUR_ESIK_AZ) {
        havaDurumu = "Saganak Yagmur";
      }
      else {
        havaDurumu = "Hava Acik";
      }
      
      delay(2000); // 2 saniye sonra hava durumunu göster
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Hava Durumu:");
      lcd.setCursor(0, 1);
      lcd.print(havaDurumu);
      
      // Röle durumlarını da göster
      delay(2000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Lamba: " + String(roleLambaDurum ? "Acik" : "Kapali"));
      lcd.setCursor(0, 1);
      lcd.print("Gaz: " + String(roleGazVanasiDurum ? "Acik" : "Kapali"));
    }
  }
}

// Buzzer kontrol
void buzzerKontrol() {
  if (depremDurumu || gazKacagiDurumu || yanginDurumu || mesafeAlarmDurumu) {
    // Her 1 saniyede bir alarm sesi çıkar
    if (simdikiZaman - sonBuzzerZamani > 1000) {
      sonBuzzerZamani = simdikiZaman;
      tone(BUZZER_PIN, 2000, 300); // 2kHz frekans, 300ms süre
    }
  }
}