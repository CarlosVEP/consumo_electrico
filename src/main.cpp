#include <DHT.h>
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
#define SWITCH_DORMITORIO 33

// configuración interruptor cocina
#define SWITCH_COCINA 32

// configuración interruptor sala
#define SWITCH_SALA 35

// configuración interruptor baño
#define SWITCH_BANIO 34

// configuración pin dht
#define DHT_PIN 25

// configuración pin DHT22
constexpr char DHTTYPE = DHT22;
DHT dht(DHT_PIN,DHTTYPE);

const int LUZ_W = 12;
int totalConsumo = 0;

void setup() {
  Serial.begin(115200);
  dht.begin();

  // asignando los switchs a los pines
  pinMode(SWITCH_DORMITORIO, INPUT_PULLUP);
  pinMode(SWITCH_COCINA, INPUT_PULLUP);
  pinMode(SWITCH_SALA, INPUT_PULLUP);
  pinMode(SWITCH_BANIO, INPUT_PULLUP);
}

void loop() {
  // obteniendo valores de los potenciómetros
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
  int luzDormitorio = switchDormitorio ? LUZ_W:0;
  Serial.println(switchDormitorio? "Luz de dormitorio encendido":"Dormitorio apagado");

  // monitorización de consumo eléctrico de la cocina
  int switchCocina = digitalRead(SWITCH_COCINA);
  int luzCocina = switchCocina? LUZ_W:0;
  Serial.println(switchCocina? "Luz de cocina encendido":"Cocina apagada");
  
  // monitorización de consumo eléctrico de la sala
  int switchSala = digitalRead(SWITCH_SALA);
  int luzSala = switchSala? LUZ_W:0;
  Serial.println(switchSala? "Luz de sala encendida":"Sala apagado");

  // monitorización de consumo eléctrico del baño
  int switchBanio = digitalRead(SWITCH_BANIO);
  int luzBanio = switchBanio? LUZ_W:0;
  Serial.println(switchBanio? "Luz de baño encendido":"Baño apagado");

  totalConsumo = totalConsumo + ledTV + ledLavadora + ledRadio + ledFreezer + luzDormitorio + luzSala + luzCocina + luzBanio;
  float consumoEnKW = static_cast<float>(totalConsumo) / 1000;
  Serial.println("Consumo por segundo en kW: "+String(consumoEnKW)); // para totalConsumo 372 sale 0.37

  float temperatura = dht.readTemperature();
  if(!isnan(temperatura)){
    Serial.print("Temperatura actual:");
    Serial.println(temperatura);
    if (temperatura > 50){
      Serial.println("Envío de alerta al telegram");
    }
  }

  delay(1000);
}