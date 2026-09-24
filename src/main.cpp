#include <Arduino.h>
#include <DHT.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>

// configuración para tv
#define TV_PIN 34
#define LED_TV_PIN 13

// configuración para lavadora
#define LAVADORA_PIN 35
#define LED_LAVADORA_PIN 12

// configuración para radio
#define RADIO_PIN 32
#define LED_RADIO_PIN 14

// configuración para freezer
#define FREEZER_PIN 33
#define LED_FREEZER_PIN 27

// configuración interruptor dormitorio
#define SWITCH_DORMITORIO 4

// configuración interruptor cocina
#define SWITCH_COCINA 0

// configuración interruptor sala
#define SWITCH_SALA 2

// configuración interruptor baño
#define SWITCH_BANIO 15

// configuración pin dht
#define DHT_PIN 25

// configuración pin DHT22
constexpr char DHTTYPE = DHT22;
DHT dht(DHT_PIN,DHTTYPE);

const int LUZ_W = 12;
int totalConsumo = 0;

// Grove OLED SH1107
// 128 x 128 píxeles, I2C
U8G2_SH1107_128X128_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE);

std::string mensajes[10];

const int cantidadTextos = sizeof(mensajes) / sizeof(mensajes[0]);
int y = 12;

// Configuración de wifi
#define ssid "Wokwi-GUEST"
#define password ""

// Credenciales Telegram
const String BOT_TOKEN = "8801456979:AAFQwJqB0voh62bCDstGjjoOVFPHdgjzQ1M";
const String CHAT_ID = "519918100";

const unsigned long SENSOR_INTERVAL = 100;
const unsigned long DHT_INTERVAL = 2000;
const unsigned long OLED_INTERVAL = 500;
const unsigned long TELEGRAM_INTERVAL = 1000;
unsigned long lastSensor = 0;
unsigned long lastDHT = 0;
unsigned long lastOLED = 0;
unsigned long lastTelegram = 0;

String msjBot;
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);
bool msjBotEnviado = false;

bool releON = true;

void setup() {
  Serial.begin(115200);

  // conectar wifi
  Serial.print("Conectando a wifi ...");
  WiFi.begin(ssid, password);

  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado al wifi");
  dht.begin();

  // asignando los switchs a los pines
  pinMode(SWITCH_DORMITORIO, INPUT_PULLUP);
  pinMode(SWITCH_COCINA, INPUT_PULLUP);
  pinMode(SWITCH_SALA, INPUT_PULLUP);
  pinMode(SWITCH_BANIO, INPUT_PULLUP);

  Wire.begin(21, 22);

  oled.begin();
  oled.setFont(u8g2_font_6x10_tf);

  oled.clearBuffer();
}

// problema con type callback_query solo acepta type message
void handleNewMessages(int numNewMessages){
  // for(int i = 0; i < numNewMessages; i++){

  //     Serial.println("===== MENSAJE =====");

  //     Serial.print("Tipo: ");
  //     Serial.println(bot.messages[i].type);

  //     Serial.print("Texto: ");
  //     Serial.println(bot.messages[i].text);

  //     Serial.print("Chat ID: ");
  //     Serial.println(bot.messages[i].chat_id);

  //     Serial.println("===================");
  // }
  for(int i=0; i<numNewMessages; i++){
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;
    String type = bot.messages[i].type;
    //Verificación de que el mensaje proviene de un boton(Callback Query)
    if(type == "callback_query"){
      if(text == "APAGAR_RELE"){
        releON = false;
        bot.sendMessage(chat_id, "Se apagó el relé", "");
      }
    }
  }
  // for(int i = 0; i < numNewMessages; i++){
  //   String chat_id = String(bot.messages[i].chat_id);
  //   String text = bot.messages[i].text;
  //   String type = bot.messages[i].type;

  //   Serial.println("===== MENSAJE / ACCIÓN =====");
  //   Serial.print("Tipo: "); Serial.println(type);
  //   Serial.print("Texto: "); Serial.println(text);
  //   Serial.println("============================");

  //   // Capturar la respuesta del botón inline
  //   if(type == "callback_query"){
  //     if(text == "APAGAR_RELE"){
  //       releON = false;
  //       bot.sendMessage(chat_id, "Se apago el rele", "");
  //     }
  //   }
    
  //   // Capturar mensajes de texto tradicionales (opcional)
  //   if(type == "message"){
  //     if(text == "Hola"){
  //       bot.sendMessage(chat_id, "Hola, sistema activo", "");
  //     }
  //   }
  // }
}

void loop() {

  unsigned long tiempoTranscurrido = millis();

  if (tiempoTranscurrido - lastSensor >= SENSOR_INTERVAL){
    lastSensor = tiempoTranscurrido;
    // obteniendo valores de los potenciómetros
    uint32_t tv = analogRead(TV_PIN);
    uint32_t lavadora = analogRead(LAVADORA_PIN);
    uint32_t radio = analogRead(RADIO_PIN);
    uint32_t freezer = analogRead(FREEZER_PIN);
  
    // monitorización de consumo de la televicion
    //Serial.println("consumo televisión: "+String(tv));
    mensajes[0] = "consumo tv: "+std::to_string(tv);
    int ledTV = map(tv, 0, 4095, 0, 255);
    analogWrite(LED_TV_PIN, ledTV);
    
    // monitorización de consumo de la lavadora
    //Serial.println("consumo lavadora: "+String(lavadora));
    mensajes[1] = "consumo lavadora: "+std::to_string(lavadora);
    int ledLavadora = map(lavadora, 0, 4095, 0, 255);
    analogWrite(LED_LAVADORA_PIN, ledLavadora);
  
    // monitorización de consumo de la radio
    //Serial.println("consumo radio: "+String(radio));
    mensajes[2] = "consumo radio: "+std::to_string(radio);
    int ledRadio = map(radio, 0, 4095, 0, 255);
    analogWrite(LED_RADIO_PIN, ledRadio);
    
    // monitorización de consumo de la freezer
    //Serial.println("consumo freezer: "+String(freezer));
    mensajes[3] = "consumo freezer: "+std::to_string(freezer);
    int ledFreezer = map(freezer, 0, 4095, 0, 255);
    analogWrite(LED_FREEZER_PIN, ledFreezer);
  
    // monitorización de consumo de la dormitorio
    int switchDormitorio = digitalRead(SWITCH_DORMITORIO);
    int luzDormitorio = switchDormitorio ? LUZ_W:0;
    //Serial.println(switchDormitorio? "Luz dormitorio encendido":"Dormitorio apagado");
    mensajes[4] = switchDormitorio? "Luz dormitorio encendido":"Dormitorio apagado";
  
    // monitorización de consumo eléctrico de la cocina
    int switchCocina = digitalRead(SWITCH_COCINA);
    int luzCocina = switchCocina? LUZ_W:0;
    //Serial.println(switchCocina? "Luz cocina encendida":"Cocina apagada");
    mensajes[5] = switchCocina? "Luz cocina encendida":"Cocina apagada";
    
    // monitorización de consumo eléctrico de la sala
    int switchSala = digitalRead(SWITCH_SALA);
    int luzSala = switchSala? LUZ_W:0;
    //Serial.println(switchSala? "Luz sala encendida":"Sala apagada");
    mensajes[6] = switchSala? "Luz sala encendida":"Sala apagada";
  
    // monitorización de consumo eléctrico del baño
    int switchBanio = digitalRead(SWITCH_BANIO);
    int luzBanio = switchBanio? LUZ_W:0;
    //Serial.println(switchBanio? "Luz baño encendido":"Baño apagado");
    mensajes[7] = switchBanio? "Luz baño encendido":"Baño apagado";
    totalConsumo = totalConsumo + ledTV + ledLavadora + ledRadio + ledFreezer + luzDormitorio + luzSala + luzCocina + luzBanio;
    float consumoEnKW = static_cast<float>(totalConsumo) / 1000;
    Serial.println("Consumo en kW/s: "+String(consumoEnKW)); // para totalConsumo 372 sale 0.37
    mensajes[8] = "Consumo en kW/s: " + std::to_string(consumoEnKW);
  }

  if (tiempoTranscurrido - lastDHT >= DHT_INTERVAL){
    lastDHT = tiempoTranscurrido;
    float temperatura = dht.readTemperature();
    if(!isnan(temperatura)){
      Serial.print("Temperatura: ");
      Serial.println(temperatura);
      mensajes[9] = "Temperatura: " + std::to_string(temperatura);
      if (temperatura > 50){
        if (!msjBotEnviado){
          Serial.println("Envío de alerta al telegram");
          String keyboardJson = "[[{\"text\":\"Apagar relé\",\"callback_data\":\"APAGAR_RELE\"}]]";
          bot.sendMessageWithInlineKeyboard(CHAT_ID, "🚨 Temperatura inusualmente alta", "Markdown", keyboardJson, 0);
          msjBotEnviado = true;
        } else {
          msjBotEnviado = false;
        }
      }
    }
  }

  if(tiempoTranscurrido - lastTelegram >= TELEGRAM_INTERVAL){
    lastTelegram = tiempoTranscurrido;
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while(numNewMessages){
      Serial.println("Procesando nuevos mensajes...");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
  }

  if (tiempoTranscurrido - lastOLED >= OLED_INTERVAL){
    lastOLED = tiempoTranscurrido;
    oled.clearBuffer();
    int posicionY = 12;
    // recorrido del arreglo para mostrarlo en el oled
    for (int i = 0; i < cantidadTextos; i++) {
      oled.drawStr(0, posicionY, mensajes[i].c_str());
      posicionY += 12;
      if (posicionY > 124) {
        break;
      }
    }
    oled.sendBuffer();
  }
}