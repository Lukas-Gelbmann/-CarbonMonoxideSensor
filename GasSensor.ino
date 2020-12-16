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
#define MQTT_USER "gassensor"
#define MQTT_PW "pw"
WiFiClient net;
MQTTClient client;


// Wifi 
char ssid[] = SECRET_SSID;            
char pass[] = SECRET_PASS;   
WiFiSSLClient wificlient;


// Telegram BOT
const char BotToken[] = SECRET_BOT_TOKEN;
TelegramBot bot (BotToken, wificlient);


// logic vars
int threshold = 200;
bool reset = true;


void setup(){
  //serial setup
  Serial.begin(115200);
  while (!Serial) {}
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
  //co data logic
  int sum = 0;
  for(int i = 0; i < 50; i++){
    sum = sum + analogRead(A1);
    delay(500);
  }
  int current = sum/50;
  Serial.print("Current value: ");
  Serial.println(current, DEC);


  //mqtt publish logic
  client.publish("KohlenstoffmonoxidFruehwarnsystem/Test",  String(current));
  

  //thingspeak logic
  ThingSpeak.setField(1,current);
  int ret=ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  Serial.println("ThingSpeak returned: "+String(ret));


  //reset logic
  message m = bot.getUpdates();
  if(m.text == "reset"){
    bot.sendMessage("1240867501", "Alert reset");
    reset = true;
  }
  

  //send message logic
  if(current > threshold && reset) {
    reset = false;
    bot.sendMessage("1240867501", "Alert");
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
