/*
 * Requires Francisco Malpartida's LCD library: https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads/
 * (Download the zip file, then Sketch -> Include Library -> Add .ZIP Library)
 * 
 */

#include <Arduino.h>
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>

// Rotary encoder stuff
volatile int lastEncoded = 0;
volatile long encoderValue = 0;
long lastencoderValue = 0;
int lastMSB = 0;
int lastLSB = 0;

// LCD Disply Stuff
#define I2C_ADDR    0x27 // <<----- Add your address here.  Find it from I2C Scanner
#define BACKLIGHT_PIN     3
#define En_pin  2
#define Rw_pin  1
#define Rs_pin  0
#define D4_pin  4
#define D5_pin  5
#define D6_pin  6
#define D7_pin  7
LiquidCrystal_I2C  lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin);

// Stuff for analogue pots
int prevVals[3];
int minPotVar = 3;
String prevMsg = "";

void setup() {
  // LCD screen setup
  lcd.begin (16,2);
  lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.home (); // go home  
  lcd.print("<< Hivemaker >>");  

  // Rotary encoder setup
  digitalWrite(20, HIGH); 
  digitalWrite(21, HIGH); 

  // Analogue pot setup
  prevVals[0] = -99;
  prevVals[1] = -99;
  prevVals[2] = -99;
  pinMode(14, OUTPUT);
  pinMode(15, OUTPUT);
  pinMode(16, OUTPUT);
  pinMode(17, OUTPUT);
  pinMode(18, OUTPUT);
  pinMode(19, OUTPUT);

  Serial.begin(9600);
}

void loop() {
  digitalWrite(14, HIGH);
  digitalWrite(15, LOW);
  digitalWrite(16, HIGH);
  digitalWrite(17, LOW);
  digitalWrite(18, HIGH);
  digitalWrite(19, LOW);
  int val1 = 1024 - analogRead(A9); // Correct for stupid backwards wiring
  int val2 = analogRead(A5);
  int val3 = 1024 - analogRead(A1); // Correct for stupid backwards wiring
  if(abs(prevVals[0]  - val1) > minPotVar){
    prevVals[0] = val1;
  }
  if(abs(prevVals[1]  - val2) > minPotVar){
    prevVals[1] = val2;
  }
  if(abs(prevVals[2]  - val3) > minPotVar){
    prevVals[2] = val3;
  }
  String msg = String(prevVals[0]) + "," + String(prevVals[1]) + "," + String(prevVals[2]) + "*";
  if(msg != prevMsg){
    Serial.print(msg);
    prevMsg = msg;
    //Serial.print("\n");
   lcd.setCursor (0,1);
   lcd.print(msg);
  }


  int MSB = digitalRead(20); //MSB = most significant bit
  int LSB = digitalRead(21); //LSB = least significant bit
  
  int encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number 
  int sum = (lastEncoded << 2) | encoded; //adding it to the previous encoded value 
  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue ++; 
  if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue --; 
  if(encoded != lastEncoded){
    Serial.println(encoderValue);
  }
  lastEncoded = encoded;

}



