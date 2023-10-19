#include <Servo.h>
#include <LiquidCrystal.h>

Servo Servo1;
LiquidCrystal lcd(8,9,10,11,12,13);

//Pin number for all outputs except the LCD
const int led1 = 2;
const int led2 = 3;
const int led3 = 4;
const int servo = 5;
const int speaker = 6;
  
int mode = 1;                                     //Type of message
String message = "";

unsigned long waveTime = 5000;                    //Duration of handwave
unsigned long speakerTime = 4000;                 //Duration of speaker noise

unsigned long lastWaveSequence = 0;               
unsigned long lastTuneSequence = 0;

unsigned long currentMillis = 0;                  //Runtime of current loop 
unsigned long lastWaveMillis = 0;                 //Runtime since last wave 
unsigned long lastBlinkMillis = 0;                //Runtime since last blink of LEDs
unsigned long lastTunePlayedMillis = 0;           //Runtime since last played tone
unsigned long lastNoTunePlayedMillis = 0;         //Runtime since last time speaker played no tone
unsigned long lastMessagePrintedMillis = 0;       //Runtime since last message printed on LCD
unsigned long lastLCDScroll = 0;                  //Runtime since last scroll of LCD
unsigned long lastAlertStartMillis = 0;           //Runtime since last alert started

unsigned long waveInterval = 600;                 //Interval between hand waves
unsigned long blinkInterval = 1000;               //Blink duration of activated LEDs
unsigned long alertInterval = 8000;               //Alert duration

unsigned long lastAlert = 0;                      //Last alert sequence

unsigned long maximumUptime = 3888000000;         //Maximum uptime before reset of 45 Days

bool handOnRight = false;                         //State tracker for servo
bool led1High = false;                            //State tracker for LED1
bool led2High = false;                            //State tracker for LED2
bool led3High = false;                            //State tracker for LED3
bool speakerPlaying = false;                      //State tracker for speaker
bool lcdShowingMessage = false;                   //State tracker for LCD
bool alerting = false;                            

int receivedNumber = 0;                           //Number received from serial communication
String receivedMessage = "";                      //String received from serial communication

void(* resetFunc) (void) = 0;                     //Built in function for resetting Arduino

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Arduino Notifier"); 

  pinMode(led1, OUTPUT); 
  pinMode(led2, OUTPUT); 
  pinMode(led3, OUTPUT); 
  pinMode(speaker, OUTPUT);

  Servo1.attach(5);

  lcd.begin(16,1);

  lcd.print("No Messages");
}

void loop() {

  currentMillis = millis();

  if(currentMillis > maximumUptime){
    resetFunc();
  }

  if(alerting) {
    if(currentMillis - lastAlert >= alertInterval) {        //If alert interval is over, set message to "" to stop alerting
      alerting = false;
      cleanUpAfterAlert();
    } else {
      alertSequence();
    }
  } else {
    if(message != "") {
      alerting = true;
      lastAlert = currentMillis;
    }
    checkForMessage();
  }

}

void alertSequence() {
  waveHand(); 
  playTune(mode);
  turnOnLights(mode);
  displayMessage(message);
}

void checkForMessage() {
  if(Serial.available() > 0) {

    String serialPacket = Serial.readStringUntil("^");
    
    char firstSymbol = serialPacket.charAt(0);

    if(firstSymbol == '|'){

      for(int i = 1; i < serialPacket.length() - 4; i++){         //Iterates one less char for the first symbol, another for the last, and one less because of the mode number, and one for newline

        char symbol = serialPacket.charAt(i);

        if(symbol == ';') {

          receivedNumber = serialPacket.charAt(i + 1) - '0';  //Casts from char to int

          Serial.print("Received String: ");
          Serial.println(receivedMessage);
          Serial.print("Received Number: ");
          Serial.println(receivedNumber);

          message = receivedMessage;
          mode = receivedNumber;

          receivedMessage = "";

        } else {
          receivedMessage += symbol;
          Serial.println("receivedMessage: " + receivedMessage); 
        }

      }

    } else {
      Serial.println(serialPacket.substring(0,serialPacket.length()-3));
    }
  }
}

void cleanUpAfterAlert() {
  message = "";
  mode = 1;
  handOnRight = false;                         
  led1High = false;                            
  led2High = false;                            
  led3High = false;                            
  speakerPlaying = false;                      
  lcdShowingMessage = false;
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  digitalWrite(led3, LOW);
  noTone(speaker);
  Servo1.write(90);
  lcd.clear();
  lcd.print("No Messages");
}

void displayMessage(String message) {
  if(lcdShowingMessage) {
    if((currentMillis - lastLCDScroll >= 300) && (message.length() > 16)) {
      lcd.scrollDisplayLeft();
      lastLCDScroll = currentMillis;
    }
  }else {
    if(currentMillis - lastMessagePrintedMillis >= 1000) {
      lcd.clear();
      lcd.print(message);
      lcdShowingMessage = true;
      lastMessagePrintedMillis = currentMillis;
    }
  }
}

void waveHand() {
  if(handOnRight){
    if(currentMillis - lastWaveMillis >= waveInterval){
      Servo1.write(0);
      handOnRight = false;
      lastWaveMillis = currentMillis;
    }
  }else{
    if(currentMillis - lastWaveMillis >= waveInterval){
      Servo1.write(180);
      handOnRight = true;
      lastWaveMillis = currentMillis;
    }
  }
}


void turnOnLights(int mode) {
    switch(mode) {
    case 1:
      if(led1High){
        if(currentMillis - lastBlinkMillis >= blinkInterval){
          digitalWrite(led1, LOW);
          led1High = false;
          lastBlinkMillis = currentMillis;
        }
      }else{
        if(currentMillis - lastBlinkMillis >= blinkInterval){
          digitalWrite(led1, HIGH);
          led1High = true;
          lastBlinkMillis = currentMillis;
        }
      }
      break;
    case 2:
      if(led2High){
        if(currentMillis - lastBlinkMillis >= blinkInterval){
          digitalWrite(led2, LOW);
          led2High = false;
          lastBlinkMillis = currentMillis;
        }
      }else{
        if(currentMillis - lastBlinkMillis >= blinkInterval){
          digitalWrite(led2, HIGH);
          led2High = true;
          lastBlinkMillis = currentMillis;
        }
      }
      break;
    case 3:
      if(led3High){
        if(currentMillis - lastBlinkMillis >= blinkInterval){
          digitalWrite(led3, LOW);
          led3High = false;
          lastBlinkMillis = currentMillis;
        }
      }else{
        if(currentMillis - lastBlinkMillis >= blinkInterval){
          digitalWrite(led3, HIGH);
          led3High = true;
          lastBlinkMillis = currentMillis;
        }
      }
      break;
    default:
      return;
    }
}

void playTune(int mode) {

    unsigned long playInterval;
    unsigned long silenceInterval;
    int frequency;

    switch(mode) {
    case 1:
      playInterval = 800;
      silenceInterval = 1000;
      frequency = 800;
      break;
    case 2:
      playInterval = 800;
      silenceInterval = 1000;
      frequency = 600;
      break;
    case 3:
      playInterval = 800;
      silenceInterval = 1000;
      frequency = 400;
      break;
    default:
      return;
    }
      if(speakerPlaying) {
        if(currentMillis - lastTunePlayedMillis >= silenceInterval){
          noTone(speaker); 
          lastTunePlayedMillis = currentMillis;
          speakerPlaying = false;
        }
      } else {
        if(currentMillis - lastNoTunePlayedMillis >= playInterval){
          tone(speaker, frequency, silenceInterval - playInterval);
          lastNoTunePlayedMillis = currentMillis;
          speakerPlaying = true;
        }
      } 
}
