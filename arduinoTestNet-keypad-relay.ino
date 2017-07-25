#include <Keypad.h>
#include <SPI.h>
#include <Ethernet.h>

const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns
char keys[ROWS][COLS] = {
        {'1','2','3','A'},
        {'4','5','6','B'},
        {'7','8','9','C'},
        {'*','0','#','D'}
        };
byte rowPins[ROWS] = {5, A4, 3, 2}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {9, 8, 7, 6}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
byte drawerPin = A5;
byte redLedPin = A3;
byte YellowLedPin = A2;
byte GreenLedPin = A1;
byte buzzer = A0;

char server[] = "10.42.0.1";
////setup ethernet
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

EthernetClient client;

boolean blink = false;
char count[4];

unsigned long timer; // the timer
unsigned long INTERVAL = 5000; // the repeat interval

void setup(){
  Serial.begin(9600);
  pinMode(drawerPin, OUTPUT);
  pinMode(redLedPin, OUTPUT);
  pinMode(YellowLedPin, OUTPUT);
  pinMode(GreenLedPin, OUTPUT);  // sets the digital pin as output
  pinMode(buzzer, OUTPUT);
  digitalWrite(redLedPin,LOW);
  digitalWrite(YellowLedPin,HIGH);
  digitalWrite(GreenLedPin,LOW);
  digitalWrite(buzzer,LOW);
  digitalWrite(drawerPin, HIGH);   // sets the LED on
  keypad.addEventListener(keypadEvent); //add an event listener for this keypad     ////set yellow led indcate device waiting status
  if(Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    digitalWrite(YellowLedPin,LOW);
    digitalWrite(redLedPin,HIGH);    /////set red led indcate fail status
  }else{
    Serial.println("started");
    Serial.println(Ethernet.localIP());
    digitalWrite(YellowLedPin,LOW);
    digitalWrite(GreenLedPin,HIGH);
    buzzerSec();
    timer = millis(); // start timer   ////Set green led indcate device working status
  }
}

void loop(){
  char key = keypad.getKey();
  Ethernet.maintain();
  if ((millis()-timer) > INTERVAL) {
      timer = millis();// reset timer by moving it along to the next interval
      if(!client.connect(server, 8080)) {
        client.stop();
        Serial.println("disconnected check server");
        digitalWrite(redLedPin,HIGH);
        digitalWrite(GreenLedPin,LOW);     /////set red led indcate fail status
      }else{
        client.stop();
        digitalWrite(redLedPin,LOW);
        digitalWrite(GreenLedPin,HIGH);        
        Serial.println("connected everything ok...");////Set green led indcate device working status
      }
    }
}

//take care of some special events
void keypadEvent(KeypadEvent key){
  switch (keypad.getState()){
    case PRESSED:
      switch (key){
        case '#': break;
        case '*': count[0] = 0; break;
        case 'A': sendInfo(count,"A"); break;
        case 'B': sendInfo(count,"B"); break;
        case 'C': sendInfo(count,"C"); break;
        case 'D': break;
        default: total(key);
        break;
      }
    break;
  }
}

void sendInfo(String N,String T){
  client.stop();
  String data="count="+N+"&type="+T;
  if (client.connect(server, 8080)) {
    client.println("POST / HTTP/1.1");
    client.println("Connection: close");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.println(data.length());
    client.println();
    client.println(data);
  }else {
    buzzerFail();
    Serial.println("connection failed");
  }

  if(client.connected()) {
    buzzerSec();
    digitalWrite(drawerPin,LOW);
    delay(100);
    digitalWrite(drawerPin,HIGH);
    client.stop();
  }
  count[0] = 0;
}

void total(char number){
  if(!(strlen(count) == 0 && number == '0')){
    if(strlen(count) <= 1){
      strncat(count, &number, 1);
      buzz();
    }
  }
}

void buzzerSec(){
  digitalWrite(buzzer,HIGH);
  delay(20);
  digitalWrite(buzzer,LOW);
  delay(5);
  digitalWrite(buzzer,HIGH);
  delay(20);
  digitalWrite(buzzer,LOW);
}

void buzzerFail(){
  digitalWrite(buzzer,HIGH);
  delay(5);
  digitalWrite(buzzer,LOW);
  delay(5);
  digitalWrite(buzzer,HIGH);
  delay(5);
  digitalWrite(buzzer,LOW);
}

void buzz(){
  digitalWrite(buzzer,HIGH);
  delay(10);
  digitalWrite(buzzer,LOW);
}