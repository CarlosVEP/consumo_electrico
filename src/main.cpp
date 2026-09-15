#include <Arduino.h>

// configuración para tv
#define TV_PIN 4
#define LED_TV_PIN 13

// configuración para lavadora
#define LAVADORA_PIN 0
#define LED_LAVADORA_PIN 12

// configuración para radio
#define RADIO_PIN 2
#define LED_RADIO_PIN 14

// configuración para freezer
#define FREEZER_PIN 15
#define LED_FREEZER_PIN 27

// configuración interruptor dormitorio
#define SWITCH_DORMITORIO 26

// configuración interruptor cocina
#define SWITCH_COCINA 25

// configuración interruptor sala
#define SWITCH_SALA 33

// configuración interruptor baño
#define SWITCH_BANIO 32

void setup() {
  Serial.begin(115200);
  //pinMode(LED_TV_PIN, OUTPUT);

  pinMode(SWITCH_DORMITORIO, INPUT_PULLUP);
  pinMode(SWITCH_COCINA, INPUT_PULLUP);
  pinMode(SWITCH_SALA, INPUT_PULLUP);
  pinMode(SWITCH_BANIO, INPUT_PULLUP);
}

void loop() {
  uint32_t tv = analogRead(TV_PIN);
  uint32_t lavadora = analogRead(LAVADORA_PIN);
  uint32_t radio = analogRead(RADIO_PIN);
  uint32_t freezer = analogRead(FREEZER_PIN);

  // monitorización de consumo eléctrico de la televicion
  Serial.println("consumo eléctrico televisión: "+String(tv));
  int ledTV = map(tv, 0, 4095, 0, 255);
  analogWrite(LED_TV_PIN, ledTV);
  
  // monitorización de consumo eléctrico de la lavadora
  Serial.println("consumo eléctrico lavadora: "+String(lavadora));
  int ledLavadora = map(lavadora, 0, 4095, 0, 255);
  analogWrite(LED_LAVADORA_PIN, ledLavadora);

  // monitorización de consumo eléctrico de la radio
  Serial.println("consumo eléctrico radio: "+String(radio));
  int ledRadio = map(radio, 0, 4095, 0, 255);
  analogWrite(LED_RADIO_PIN, ledRadio);
  
  // monitorización de consumo eléctrico de la freezer
  Serial.println("consumo eléctrico freezer: "+String(freezer));
  int ledFreezer = map(freezer, 0, 4095, 0, 255);
  analogWrite(LED_FREEZER_PIN, ledFreezer);

  // monitorización de consumo eléctrico de la dormitorio
  int switchDormitorio = digitalRead(SWITCH_DORMITORIO);
  Serial.println(switchDormitorio? "Luz de dormitorio encendido":"Dormitorio apagado");

  // monitorización de consumo eléctrico de la cocina
  int switchCocina = digitalRead(SWITCH_COCINA);
  Serial.println(switchCocina? "Luz de cocina encendido":"Cocina apagada");
  
  // monitorización de consumo eléctrico de la sala
  int switchSala = digitalRead(SWITCH_SALA);
  Serial.println(switchSala? "Luz de sala encendido":"Sala apagado");

  // monitorización de consumo eléctrico del baño
  int switchBanio = digitalRead(SWITCH_BANIO);
  Serial.println(switchBanio? "Luz de baño encendido":"Baño apagado");
  delay(500);
}