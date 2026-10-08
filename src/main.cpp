#include <Arduino.h>
#include <DHT.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "secrets.h"

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

constexpr char DHTTYPE = DHT22;
DHT dht(DHT_PIN,DHTTYPE);

// configuración OLED
#define OLED_SDA 21
#define OLED_SCL 22

const int LUZ_W = 12;
int totalConsumo = 0;
float totalConsumoKW = 0.0;
// const unsigned long T_CONSUMO_INTERVAL = 5000;
// unsigned long lastConsumo = 0;

// Grove OLED SH1107
// 128 x 128 píxeles, I2C
U8G2_SH1107_128X128_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE);

std::string mensajes[10];

const int cantidadTextos = sizeof(mensajes) / sizeof(mensajes[0]);

// Configuración de wifi
#define ssid "Wokwi-GUEST"
#define password ""

// Credenciales Telegram
const String BOT_TOKEN = TOKEN_TELEGRAM;
const String CHAT_ID = ID_TELEGRAM;
String msjBot;
WiFiClientSecure clientS;
UniversalTelegramBot bot(BOT_TOKEN, clientS);
bool msjBotEnviado = false;

// Condiguración de adafruit
#define AIO_SERVER "io.adafruit.com"
#define AIO_SERVERPORT 1883
#define AIO_USERNAME USER_ADAFRUIT
#define AIO_KEY KEY_ADAFRUIT
#define AIO_GRUPO GROUP_ADAFRUIT
// Configuración del cliente MQTT
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
// Configurar los feeds
//String feed = String(AIO_USERNAME) + "/feeds/sensor_datos";
// String feed = String(AIO_USERNAME) + "/feeds/monitoreo-inteligente.consumo-total";
// Adafruit_MQTT_Publish totalConsumoKWFeed = Adafruit_MQTT_Publish(&mqtt, feed.c_str());
// Adafruit_MQTT_Subscribe ledFeed = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/led_control");

const unsigned long SENSOR_INTERVAL = 500;
const unsigned long DHT_INTERVAL = 2000;
const unsigned long OLED_INTERVAL = 500;
const unsigned long TELEGRAM_INTERVAL = 5000;
const unsigned long ADAFRUIT_INTERVAL = 10000;

unsigned long lastSensor = 0;
unsigned long lastDHT = 0;
unsigned long lastOLED = 0;
unsigned long lastTelegram = 0;
unsigned long lastAdafruit = 0;

uint32_t ultimoTV = 0;
uint32_t ultimoLavadora = 0;
uint32_t ultimoRadio = 0;
uint32_t ultimoFreezer = 0;
int ultimoSwitchDormitorio = 0;
int ultimoSwitchCocina = 0;
int ultimoSwitchSala = 0;
int ultimoSwitchBanio = 0;
float ultimaTemperatura = 0;

bool releON = true;

void setup() {
  Serial.begin(115200);

  // conectar wifi
  Serial.print("Conectando a wifi ...");
  WiFi.begin(ssid, password);

  clientS.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  
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

  Wire.begin(OLED_SDA, OLED_SCL);

  oled.begin();
  oled.setFont(u8g2_font_6x10_tf);

  oled.clearBuffer();
}

void enviarFeed(String feedKey, float valor){
  String rutaFeed = String(AIO_USERNAME) + "/feeds/" + String(GROUP_ADAFRUIT) + feedKey;
  Adafruit_MQTT_Publish feed = Adafruit_MQTT_Publish(&mqtt, rutaFeed.c_str());
  if(!feed.publish(valor)){
    Serial.print("Error al enviar el feed: ");
    Serial.println(feedKey);
  }
  delay(150);
}
// void enviarFeed(String feedKey, String valor) {
//   WiFiClientSecure clienteSeguro;
//   clienteSeguro.setInsecure(); // evita fallos de certificado HTTPS en la simulación

//   HTTPClient http;
//   http.setTimeout(8000); // más tiempo de espera para evitar el error -1 por conexiones lentas
//   String url = "https://io.adafruit.com/api/v2/" + String(AIO_USERNAME) +
//                "/feeds/" + AIO_GRUPO + "." + feedKey + "/data";

//   http.begin(clienteSeguro, url);
//   http.addHeader("Content-Type", "application/json");
//   http.addHeader("X-AIO-Key", AIO_KEY);

//   String payload = "{\"value\":\"" + valor + "\"}";
//   int codigo = http.POST(payload);

//   if (codigo == 200 || codigo == 201) {
//     Serial.println("OK -> " + feedKey + " = " + valor);
//   } else {
//     Serial.println("Error al enviar " + feedKey + ": " + String(codigo));
//   }
//   http.end();
// }

void handleNewMessages(int numNewMessages){
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
}

void MQTT_connect(){
  int8_t ret;
  if(mqtt.connected()){
    return;
  }
  Serial.println("Conectando a MQTT ...");
  while((ret = mqtt.connect())!=0){
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Reintentando en 5 segundos ...");
    mqtt.disconnect();
    delay(5000);
  }
  Serial.println("MQTT Conectado!");
}

void loop() {
  if (releON){
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
      ultimoTV = map(tv, 0, 4095, 0, 150);
      mensajes[0] = "consumo tv: "+std::to_string(ultimoTV);
      analogWrite(LED_TV_PIN, ultimoTV);
      
      // monitorización de consumo de la lavadora
      //Serial.println("consumo lavadora: "+String(lavadora));
      ultimoLavadora = map(lavadora, 0, 4095, 0, 800);
      mensajes[1] = "consumo lavadora: "+std::to_string(ultimoLavadora);
      analogWrite(LED_LAVADORA_PIN, ultimoLavadora);
    
      // monitorización de consumo de la radio
      //Serial.println("consumo radio: "+String(radio));
      ultimoRadio = map(radio, 0, 4095, 0, 100);
      mensajes[2] = "consumo radio: "+std::to_string(ultimoRadio);
      analogWrite(LED_RADIO_PIN, ultimoRadio);
      
      // monitorización de consumo de la freezer
      //Serial.println("consumo freezer: "+String(freezer));
      ultimoFreezer = map(freezer, 0, 4095, 0, 400);
      mensajes[3] = "consumo freezer: "+std::to_string(ultimoFreezer);
      analogWrite(LED_FREEZER_PIN, ultimoFreezer);
    
      // monitorización de consumo de la dormitorio
      int switchDormitorio = digitalRead(SWITCH_DORMITORIO);
      int luzDormitorio = !switchDormitorio ? LUZ_W:0;
      //Serial.println(switchDormitorio? "Luz dormitorio encendido":"Dormitorio apagado");
      mensajes[4] = !switchDormitorio? "Luz dormitorio encendido":"Dormitorio apagado";
    
      // monitorización de consumo eléctrico de la cocina
      int switchCocina = digitalRead(SWITCH_COCINA);
      int luzCocina = !switchCocina? LUZ_W:0;
      //Serial.println(switchCocina? "Luz cocina encendida":"Cocina apagada");
      mensajes[5] = !switchCocina? "Luz cocina encendida":"Cocina apagada";
      
      // monitorización de consumo eléctrico de la sala
      int switchSala = digitalRead(SWITCH_SALA);
      int luzSala = !switchSala? LUZ_W:0;
      //Serial.println(switchSala? "Luz sala encendida":"Sala apagada");
      mensajes[6] = !switchSala? "Luz sala encendida":"Sala apagada";
    
      // monitorización de consumo eléctrico del baño
      int switchBanio = digitalRead(SWITCH_BANIO);
      int luzBanio = !switchBanio? LUZ_W:0;
      //Serial.println(switchBanio? "Luz baño encendido":"Baño apagado");
      mensajes[7] = !switchBanio? "Luz baño encendido":"Baño apagado";
      
      totalConsumo = ultimoTV + ultimoLavadora + ultimoRadio + ultimoFreezer + luzDormitorio + luzSala + luzCocina + luzBanio;
      totalConsumoKW = static_cast<float>(totalConsumo) / 1000.0;
      mensajes[8] = "Consumo en kW: " + std::to_string(totalConsumoKW);
  
      // ultimoTV = tv;
      // ultimoLavadora = lavadora;
      // ultimoRadio = radio;
      // ultimoFreezer = freezer;
      ultimoSwitchDormitorio = switchDormitorio;
      ultimoSwitchCocina = switchCocina;
      ultimoSwitchSala = switchSala;
      ultimoSwitchBanio = switchBanio;
    }
  
    if (tiempoTranscurrido - lastDHT >= DHT_INTERVAL){
      lastDHT = tiempoTranscurrido;
      float temperatura = dht.readTemperature();
      if(!isnan(temperatura)){
        mensajes[9] = "Temperatura: " + std::to_string(temperatura);
        if (temperatura > 50){
          if (!msjBotEnviado){
            Serial.println("Envío de alerta al telegram");
            String keyboardJson = "[[{\"text\":\"Apagar relé\",\"callback_data\":\"APAGAR_RELE\"}]]";
            bot.sendMessageWithInlineKeyboard(CHAT_ID, "🚨 Temperatura inusualmente alta", "", keyboardJson);
            msjBotEnviado = true;
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
  
    if (tiempoTranscurrido - lastAdafruit >= ADAFRUIT_INTERVAL){
      lastAdafruit = tiempoTranscurrido;
      MQTT_connect();
      Serial.println(mensajes[0].c_str());
      Serial.println(mensajes[1].c_str());
      Serial.println(mensajes[2].c_str());
      Serial.println(mensajes[3].c_str());
      Serial.println(mensajes[4].c_str());
      Serial.println(mensajes[5].c_str());
      Serial.println(mensajes[6].c_str());
      Serial.println(mensajes[7].c_str());
      Serial.println(mensajes[8].c_str());
      Serial.println(mensajes[9].c_str());
      // Serial.println("Enviando a adafruit");
      // totalConsumoKWFeed.publish(totalConsumoKW);
      enviarFeed("tv", ultimoTV);
      enviarFeed("lavadora", ultimoLavadora);
      enviarFeed("radio", ultimoRadio);
      enviarFeed("freezer", ultimoFreezer);
      // enviarFeed("temperatura", ultimaTemperatura);
      // enviarFeed("luz-dormitorio", ultimoSwitchDormitorio ? 1 : 0);
      // enviarFeed("luz-cocina", ultimoSwitchCocina ? 1 : 0);
      // enviarFeed("luz-sala", ultimoSwitchSala ? 1 : 0);
      // enviarFeed("luz-bano", ultimoSwitchBanio ? 1 : 0);
      enviarFeed("consumo-total", totalConsumoKW);
    }
  } else {
    ultimoTV = 0;
    ultimoLavadora = 0;
    ultimoRadio = 0;
    ultimoFreezer = 0;
    ultimoSwitchDormitorio = 0;
    ultimoSwitchCocina = 0;
    ultimoSwitchSala = 0;
    ultimoSwitchBanio = 0;
    ultimaTemperatura = 0;

    oled.clearBuffer();
    int posicionY = 12;
    oled.drawStr(0, posicionY, "APAGADO DE EMERGENCIA");
    oled.sendBuffer();
    delay(500);
  }
}