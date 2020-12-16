#include <LiquidCrystal.h> 

//// Alarmsystem implementation 

boolean alarmActive = true;

void setup() {
  // Setup LCD
  initLCD();
  outputData("1234"); 

  // Setup Buzzer 
  initBuzzer();
  
  // Setup RGB 
  initRGB();

  // Setup Button 
  initResetButton(); 
}

void loop() {
  setRgbForAlarm();
  setBuzzerForAlarm();
  checkResetButton();
}

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

void initLCD() {
   // set the brightness of the LCD screen to the maximum value 
  analogWrite(brightness, 0); 
  Serial.begin(9600);  
  lcd.begin(16, 2); // begin LCD screen with 16 columns and 2 rows
}

void outputData(String data) {
  lcd.setCursor(0,0); 
  lcd.print("Status ");
  lcd.setCursor(8,0); 
  lcd.print("OK");
  lcd.setCursor(0,1);
  lcd.print("CO ");
  lcd.setCursor(8,1);
  lcd.print(data);
}

//// Buzzer implementation ////

const int buzzerPin = 8;

void initBuzzer() {
  pinMode(buzzerPin,OUTPUT);
}

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

void initResetButton() {
  pinMode(buttonPin, INPUT_PULLUP);
  buttonState = digitalRead(buttonPin);
  Serial.begin(9600);
  pinMode(buttonPin, INPUT_PULLUP);
}

void checkResetButton() {
  // read the state of the switch/button:
  currentState = digitalRead(buttonPin);

  if(lastState == LOW && currentState == HIGH) {
    alarmActive = false;
  }

  // save the the last state
  lastState = currentState;
}

//// MQQT Client implementation ////

void receiveMessage() {
  
}
