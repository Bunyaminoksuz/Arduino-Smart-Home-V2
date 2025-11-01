#include <IRremote.h>

#define IR_RECEIVE_PIN 12  // Arduino'ya bağladığınız IR alıcı pin

IRrecv irrecv(IR_RECEIVE_PIN);
decode_results results;

void setup() {
  Serial.begin(9600);
  irrecv.enableIRIn();  // IR alıcıyı başlat
  Serial.println("IR Kumanda Kod Tespit Programi");
  Serial.println("Herhangi bir tusa basin...");
}

void loop() {
  if (irrecv.decode(&results)) {
    Serial.print("Tus Kodu (HEX): 0x");
    Serial.println(results.value, HEX);
    Serial.println("-----------------------");
    irrecv.resume();  // Sonraki değeri almak için hazırlan
    delay(300);       // Butonun bir kez algılanması için
  }
}