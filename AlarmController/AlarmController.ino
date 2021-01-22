#include <SPI.h>
#include <LiquidCrystal.h> 
#include <WiFi101.h>
#include <MQTT.h>


//// Alarmsystem implementation 

// Value to set alarm on/off
boolean alarmActive = false;

void setup() {
  // Setup LCD
  initLCD();

  // Setup Buzzer 
  initBuzzer();
  
  // Setup RGB 
  initRGB();

  // Setup Button 
  initResetButton(); 
  
  // Setup Wifi 
  if (initWifi() == 1) {
    // Setup MQTT 
    isConnected();
    delay(1000);
    initMqtt();
  }
}

void loop() {
  clientLoop();
  setRgbForAlarm();
  setBuzzerForAlarm();
  checkResetButton();
}

// Method to set alarm active of not active 
void setAlarm(boolean active) {
  alarmActive = active;
}

//// LCD implementation ////

const int brightness = A3; 
const int rs = 12, 
         en = 11, 
         d4 = 2, 
         d5 = 3, 
         d6 = 4, 
         d7 = 5; 
LiquidCrystal lcd(rs, en, d4, d5, d6, d7); 

// Setup LCD screen 
void initLCD() {
   // set the brightness of the LCD screen to the maximum value 
  analogWrite(brightness, 0); 
  Serial.begin(9600);  
  lcd.begin(16, 2); // begin LCD screen with 16 columns and 2 rows
}

// Check incoming value type to deside which status to show on LCD screen
void outputData(String data, int type) {
  clearLCD();

  switch (type) {
    case 0: 
      noConnection();
      break;
    case 1:
      showData("--", "--");
      break;
    case 2: 
      showData("DANGER", data);
      break;
    default: 
      showData("OK", data);
      break;
  }
}

// Defines basic information to display 
void showData(String status, String data) {
  lcd.setCursor(0,0); 
  lcd.print("Status ");
  lcd.setCursor(8,0); 
  lcd.print(status);
  lcd.setCursor(0,1);
  lcd.print("CO ");
  lcd.setCursor(8,1);
  lcd.print(data);
}

// Show message Wifi is not connected 
void noConnection() {
  clearLCD();
  lcd.setCursor(0,0); 
  lcd.print("Status");
  lcd.setCursor(0,1); 
  lcd.print("Wifi: FALSE");
}

// Shows message for booting process 
void isWaiting() {
  clearLCD();
  lcd.setCursor(0,0);
  lcd.print("waiting ...");
}

// Show message when try connecting to Wifi
void connecting() {
  clearLCD();
  lcd.setCursor(0,0);
  lcd.print("Try connecting");
}

// Show message if connection to Wifi was successfully 
void isConnected() {
  clearLCD();
  lcd.setCursor(0,0); 
  lcd.print("Status");
  lcd.setCursor(0,1); 
  lcd.print("Wifi: TRUE");
}

// Clear LCD screen 
void clearLCD() {
  lcd.begin(16, 2);
}

//// Buzzer implementation ////

const int buzzerPin = 8;

// Setup Buzzer 
void initBuzzer() {
  pinMode(buzzerPin,OUTPUT);
}

// Activates Buzzer 
void setBuzzerForAlarm() {
  if (alarmActive) {
    tone(buzzerPin, 31, 200);
    delay(200);
    tone(buzzerPin, 31, 400);
    delay(400);
    noTone(buzzerPin);
  } else {
    noTone(buzzerPin);  
  }
}

//// RGB Led implementation //// 

int redPin= 9;
int greenPin = 7;
int bluePin = 6;

// Setup RGB 
void initRGB() {
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
}

void setRedColor() {
  setColor(255,0,0);
}

void setBlueColor() {
  setColor(0,0,255);
}

void setGreenColor() {
    setColor(0,255,0);
}

void setColor(int redValue, int greenValue, int blueValue) {
  analogWrite(redPin, redValue);
  analogWrite(greenPin, greenValue);
  analogWrite(bluePin, blueValue);
}

// Set RGB to see if alarm is active or not
void setRgbForAlarm() {
  if (alarmActive) {
    setRedColor();
  } else {
    setGreenColor();
  }
}

//// Reset-button implementation ////

int buttonPin = A1; 
int lastState = HIGH; 
int currentState; 
int buttonState; 

// Setup Reset Button 
void initResetButton() {
  pinMode(buttonPin, INPUT_PULLUP);
  buttonState = digitalRead(buttonPin);
  Serial.begin(9600);
  pinMode(buttonPin, INPUT_PULLUP);
}

// Check reset button if pressed 
void checkResetButton() {
  // read the state of the switch/button:
  currentState = digitalRead(buttonPin);

  if(lastState == LOW && currentState == HIGH) {
    alarmActive = false;
  }

  // save the the last state
  lastState = currentState;
}

//// Wifi implementation ////
char ssid[] = "xd";
char pass[] = "Omegalul";
 
WiFiSSLClient wificlient;

// Setup Wifi connection 
int initWifi() {
  Serial.begin(115200);
  Serial.print("Connecting Wifi: ");
  connecting();
  Serial.println(ssid);
  

  int i = 0; 
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
    i++;

    if (i > 10) {
      outputData("", 0);
      return 0;
    }
  }
  Serial.println("WiFi connected");
  return 1;
}

//// MQTT Client implementation ////

// MQTT
//#define BROKER_IP "192.168.0.150"
#define BROKER_IP "broker.hivemq.com"
#define BROKER_PORT 1883
#define DEV_NAME "alarm-controller"
#define MQTT_USER "username"
#define MQTT_PW "password"
WiFiClient net;
MQTTClient client;

// Connection process for MQTT 
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

  client.subscribe("KohlenstoffmonoxidFruehwarnsystem/Test");
  outputData("", 1);
}

// Setup MQTT conntection
void initMqtt() {
  WiFi.begin(ssid, pass);
  
  client.begin(BROKER_IP, BROKER_PORT, net);
  client.onMessage(messageReceived);

  connect();
}

// Loop connection process over time
void clientLoop() {
  client.loop();

  if (!client.connected()) {
    connect();
  }
}

// Method to receive message from MQTT Broker
void messageReceived(String &topic, String &payload) {
  outputData(payload, 3);
  int value = payload.toInt();
  
  if (value > 200) {
    setAlarm(true);
    outputData(payload, 2);
  } else if (alarmActive == true) {
    setAlarm(false);
  }
}
