#include "arduino_secrets.h"
#include <WiFi101.h>
#include <SPI.h>
#include <TelegramBot.h> 
#include <MQTT.h>
#include "ThingSpeak.h"


// ThingSpeak
unsigned long myChannelNumber = 1261264;
const char * myWriteAPIKey = "PD7B0YH0W9PU8PUB";


// MQTT
#define BROKER_IP "broker.hivemq.com"
#define BROKER_PORT 1883
#define DEV_NAME "am"
#define MQTT_USER "gassensor1"
#define MQTT_PW "pw1"
WiFiClient net;
MQTTClient client;


// Wifi 
char ssid[] = SECRET_SSID;            
char pass[] = SECRET_PASS;   
WiFiSSLClient wificlient;


// Telegram BOT
const char BotToken[] = SECRET_BOT_TOKEN;
TelegramBot bot (BotToken, wificlient);
String telegramChatId = "";

// logic vars
int threshold = 100;
bool reset = true;



void setup(){
  //serial setup
  Serial.begin(115200);
  delay(1000);

  //wifi setup
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("WiFi connected");

  //mqtt setup
  client.begin(BROKER_IP, BROKER_PORT, net);
  connect();
  
  //thingspeak setup
  ThingSpeak.begin(net);

  //telegram setup
  bot.begin();
}



void loop(){
  int current = getCurrentValue();
  mqttLogic(current);
  thingspeakLogic(current);
  telegramLogic(current);
}


int getCurrentValue() {
  //co data logic
  int sum = 0;
  for(int i = 0; i < 20; i++){
    sum = sum + analogRead(A1);
    delay(500);
  }
  int current = sum/50;
  Serial.print("Current value: ");
  Serial.println(current, DEC);
  return current;
}

void mqttLogic(int current) {
  //mqtt publish logic
  client.loop();
  if (!client.connected()) {
    connect();
  }
  client.publish("KohlenstoffmonoxidFruehwarnsystem/Test",  String(current));
}

void thingspeakLogic(int current) {
  //thingspeak logic
  ThingSpeak.setField(1,current);
  int ret=ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  Serial.println("ThingSpeak returned: "+String(ret));
}


void telegramLogic(int current){
  //reset logic
  message m = bot.getUpdates();
  if(m.text == "/reset" && telegramChatId != ""){
    bot.sendMessage(telegramChatId, "Alert reset");
    reset = true;
  }
  if(m.text == "/start"){
    telegramChatId = m.chat_id;
    bot.sendMessage(telegramChatId, "You can now receive alerts");
    reset = true;
  }
  if(m.text == "/help"){
    bot.sendMessage(m.chat_id, "You can start to get alerts after typing /start\nYou can reset the alert with /reset\nWith /current you can receive the current value");
  }
  if(m.text == "/current"){
    bot.sendMessage(m.chat_id, String(current));
  }
  
  //send message logic
  if(current > threshold && reset && telegramChatId != "") {
    reset = false;
    bot.sendMessage(telegramChatId, "Alert!!!\nThe CO value is too high!");
    Serial.println("Alert");
  } else {
    Serial.println("No alert");
  }
}


void connect() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.print("\nconnecting...");
  while (!client.connect(DEV_NAME, MQTT_USER, MQTT_PW)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nconnected!");
}