#include <Arduino.h>
#line 1 "C:\\Users\\Rich C\\Documents\\Processing\\controllerTest\\controllerTest.ino"
#line 1 "C:\\Users\\Rich C\\Documents\\Processing\\controllerTest\\controllerTest.ino"
int prevVals[3];
int minPotVar = 3;
String prevMsg = "";

#line 4 "C:\\Users\\Rich C\\Documents\\Processing\\controllerTest\\controllerTest.ino"
void setup();
#line 17 "C:\\Users\\Rich C\\Documents\\Processing\\controllerTest\\controllerTest.ino"
void loop();
#line 4 "C:\\Users\\Rich C\\Documents\\Processing\\controllerTest\\controllerTest.ino"
void setup() {
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
  }
}



