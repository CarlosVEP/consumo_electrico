#include <Arduino.h>
#define TV_PIN 4
#define LED_TV_PIN 13

#define LAVADORA_PIN 0
#define LED_LAVADORA_PIN 12

#define RADIO_PIN 2
#define LED_RADIO_PIN 14

#define FREEZER_PIN 15
#define LED_FREEZER_PIN 27

void setup() {
  Serial.begin(115200);
  pinMode(LED_TV_PIN, OUTPUT);
}

void loop() {
  uint32_t tv = analogRead(TV_PIN);
  uint32_t lavadora = analogRead(LAVADORA_PIN);
  uint32_t radio = analogRead(RADIO_PIN);
  uint32_t freezer = analogRead(FREEZER_PIN);

  Serial.println("Voltaje televisión: "+String(tv));
  int ledTV = map(tv, 0, 4095, 0, 255);
  analogWrite(LED_TV_PIN, ledTV);
  
  Serial.println("Voltaje lavadora: "+String(lavadora));
  int ledLavadora = map(lavadora, 0, 4095, 0, 255);
  analogWrite(LED_LAVADORA_PIN, ledLavadora);

  Serial.println("Voltaje radio: "+String(radio));
  int ledRadio = map(radio, 0, 4095, 0, 255);
  analogWrite(LED_RADIO_PIN, ledRadio);
  
  Serial.println("Voltaje freezer: "+String(freezer));
  int ledFreezer = map(freezer, 0, 4095, 0, 255);
  analogWrite(LED_FREEZER_PIN, ledFreezer);

  delay(500);
}