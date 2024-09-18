#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

Servo servo;
LiquidCrystal_I2C lcd(0x27, 16, 2);

const int ZeroAngle = 90 - 10;
const int SingleAngle = ZeroAngle + 6;
const int LoadAngle = ZeroAngle - 14;
const int StopAngle = ZeroAngle - 3;

const int tClose = 130;
const int tOpen = 170;
const int tLoadOpen = 600;
const int tPreload = 3000;
const int tPreCont = 200;
const int VibrateTime = 500;
const int tWait = 12000;  //if no additional in this time, end

const int pLaser = 6;
const int pBuzzer = 7;
const int pMotor = 4;
const int pLine = 3;
const int pRed = 11;
const int pGreen = 12;
const int pBlue = 13;



int timer;
int loadtimer;
int mode = 0;
int LoadState;
int LoadRepeat;
int remtime;


int motorRun;
int needMotor;
int LoadingMode;

int LightSensor;
int Counter;
int Next;
int zeromax;


void setup() {
  pinMode(A0, INPUT);  //lightsense
  pinMode(6, OUTPUT);  //laser
  pinMode(7, OUTPUT);  //buzzer
  pinMode(4, OUTPUT);  //motor-relay
  pinMode(3, INPUT);  //white sense

  pinMode(11, OUTPUT); //RGB RED
  pinMode(12, OUTPUT); // RGB GREEN
  pinMode(13, OUTPUT); // RGB BLUE

  analogWrite(pRed, 0);
  analogWrite(pGreen, 0);
  analogWrite(pBlue, 255);
  
  servo.attach(5);  //servo

  servo.write(ZeroAngle);
  delay(1000);
  lcd.begin();
  lcd.backlight();
  lcd.clear();
  lcd.print("Starting...");


  Serial.begin(9600);

  digitalWrite(pLaser, HIGH);
  delay(100);
  LoadingMode = 1;


  zeromax = analogRead(A0);
  if (zeromax > 200) {

    //Red
    analogWrite(pRed, 255);
    analogWrite(pGreen, 0);
    analogWrite(pBlue, 0);

    lcd.clear();
    lcd.print("Error!");
    lcd.setCursor(0, 1);
    lcd.print("Please callibrate the light!");
    lcd.setCursor(0, 0);
    badsound();
    while (1){
        //hold
    }
  } else {
    lcd.clear();
    lcd.print("Light sensor");
    lcd.setCursor(0, 1);
    lcd.print("is good!");
    goodsound();
  }
  lcd.clear();
  lcd.print("Counting...");
  lcd.setCursor(0, 1);
  lcd.print(Counter);
}  //END SETUP

void loop() {
  delay(1);
  remtime += 1;

  LightSensor = analogRead(A0);

  if (LightSensor > zeromax + 100) {
    if (Next == 0) {
      Counter += 1;
      remtime = 0;
      lcd.setCursor(0, 1);
      lcd.print(Counter);
    }
    Next = 1;
  } else {
    Next = 0;
  }
  //PossibleBlock
  if (remtime > tWait / 2 && remtime < 4*(tWait/5) && Counter != 0){
    digitalWrite(pMotor, HIGH);
  }


  //END
  if (remtime > tWait) {
    if (Counter == 0) {

    } else {
      //Blue
      analogWrite(pRed, 0);
      analogWrite(pGreen, 0);
      analogWrite(pBlue, 255);

      servo.write(ZeroAngle);
      digitalWrite(pMotor, LOW);
      lcd.setCursor(0, 0);
      lcd.print("Final Count:");
      int k = 1;
      while (k > 0) {
        if (k < 5) {
          goodsound();
        }
        delay(1000);
        k += 1;
      }
    }
  }

  //--SERVO--
  timer += 1;
  loadtimer += 1;

  if (loadtimer == tPreload && LoadingMode == 0) {
    LoadingMode = 1;
    loadtimer = 0;
    servo.write(StopAngle);
  }

  //--Servo For Single Seed
  if (LoadingMode == 0) {

    if (remtime < tWait / 2){
      //Green
      analogWrite(pRed, 0);
      analogWrite(pGreen, 255);
      analogWrite(pBlue, 0);
    }else{
      //Orange
      analogWrite(pRed, 255);
      analogWrite(pGreen, 165);
      analogWrite(pBlue, 0);
    }

    if (mode == 0 && timer == tOpen) {
      servo.write(ZeroAngle);
      mode = 1;
      timer = 0;
    }
    if (mode == 1 && timer == tClose) {
      servo.write(SingleAngle);
      mode = 0;
      timer = 0;
    }
  } else {
    //--Servo for Loading Seeds

    //Yellow
    analogWrite(pRed, 255);
    analogWrite(pGreen, 220);
    analogWrite(pBlue, 0);

    if (LoadState==0 && digitalRead(3)==HIGH){
     digitalWrite(pMotor, HIGH);
     LoadState=3;
     timer = tLoadOpen-VibrateTime;
    }

    if (LoadState == 0) {
      //Start Vibrate, Valve High Level         ;yes shaking, no pass
      digitalWrite(pMotor, HIGH);
      servo.write(StopAngle);
      LoadState = 1;
    }
    if (timer == VibrateTime && LoadState == 1) {
      //Stop Vibrate                            ;no shaking
      digitalWrite(pMotor, LOW);
      LoadState = 2;
      timer = 0;
    }
    if (LoadState == 2) {
      //Valve SuperHigh Level                   ;loading
      servo.write(LoadAngle);
      LoadState = 3;
      timer = 0;
    }
    if (LoadState == 3 && timer == tLoadOpen) {
      //Valve High level                        ;no pass, no load
      servo.write(StopAngle);
      digitalWrite(pMotor, LOW);                   //stop vibrate if from LoadState 0
      //queue is full
      if (digitalRead(3)==HIGH || LoadRepeat == 2) {   //queue full
        LoadState = 4;
        LoadRepeat = 0;
      } else {
        LoadState = 0;
        LoadRepeat += 1;
      }
      timer = 0;
    }
    if (LoadState == 4 && timer == tPreCont) {
      LoadState = 0;
      timer = 0;
      LoadingMode = 0;
      loadtimer = 0;
    }
  }

}  //END loop

void goodsound() {
  for (int i = 0; i < 27; i++) {
    digitalWrite(pBuzzer, HIGH);
    delay(14);
    digitalWrite(pBuzzer, LOW);
    delay(14);
  }
}

void badsound() {
  for (int j = 0; j < 4; j++) {
    for (int i = 0; i < 15; i++) {
      digitalWrite(pBuzzer, HIGH);
      delay(3);
      digitalWrite(pBuzzer, LOW);
      delay(3);
    }
    delay(150);
  }
}
