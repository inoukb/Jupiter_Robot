/*
Jupiter: the orange robot
- Sony PlayStation 2 Dualshock Controller
- MD22 - 24V 5A Dual H-Bridge Motor Driver
- Micro Maestro 6-Channel USB Servo Controller
http://jrbot.blogspot.com - 2013
*/

#include <PS2X_lib.h>
#include <SoftwareSerial.h>
#include <Wire.h>

// PS2 controller
PS2X ps2x;
int error = 0; 
byte type = 0;
byte vibrate = 0;
// holds previous button state
int servoL1 = 0;
int servoR1 = 0;
int baloon = 0;
// MD22 high power motor driver
#define md22Address 0x58       // address of md 22 (all mode switches on)
#define softReg 0x07           // Byte for reading software register
#define motor1 0x01            // Byte for first motor
#define motor2 0x02            // Byte for second motor
#define accelReg 0x03          // Byte to set acelleration
byte speed1s = 128;            // Temporary variable for stick readings values
byte speed2s = 128;            // Temporary variable for stick readings values

SoftwareSerial servoSerial(2, 3); // 2 = RX et 3 = TX

void setup() {
  // Baloon
  pinMode(4, OUTPUT);
  // MD22 high power motor driver
  Wire.begin();
  delay(100);                              
  getMd22Version();                        
  setMd22Mode();                           
  Wire.beginTransmission(md22Address); // Set first motor to speed 0
  Wire.write(motor1);
  Wire.write(128);
  Wire.endTransmission();
  Wire.beginTransmission(md22Address); // Set second motor to speed 0
  Wire.write(motor2);
  Wire.write(128);
  Wire.endTransmission();
  // Serial debug
  Serial.begin(57600);
  // Maestro servo controller
  pinMode(2, INPUT);
  digitalWrite(3, HIGH); // Serial is kept high
  pinMode(3, OUTPUT);
  servoSerial.begin(9600);
  delay(5);
  servoSerial.write(0xAA);
  // PS2 Controller init
  error = ps2x.config_gamepad(13,11,10,12, true, true); // GamePad(clock, command, attention, data, Pressures?, Rumble?)
  if (error == 0) {
    Serial.println("Found Controller, configured successful");
    Serial.println("Try out all the buttons, X will vibrate the controller, faster as you press harder;");
    Serial.println("holding L1 or R1 will print out the analog stick values.");
    Serial.println("Go to www.billporter.info for updates and to report bugs.");
  }
  else if (error == 1) {
    Serial.println("No controller found, check wiring, see readme.txt to enable debug. visit www.billporter.info for troubleshooting tips"); 
  }
  else if (error == 2) {
    Serial.println("Controller found but not accepting commands. see readme.txt to enable debug. Visit www.billporter.info for troubleshooting tips");
  }
  else if (error == 3) {
    Serial.println("Controller refusing to enter Pressures mode, may not support it. ");
  }
  //Serial.print(ps2x.Analog(1), HEX);
  type = ps2x.readType(); 
  switch (type) {
    case 0:
      Serial.println("Unknown Controller type");
      break;
    case 1:
      Serial.println("DualShock Controller Found");
      break;
    case 2:
      Serial.println("GuitarHero Controller Found");
      break;
  }
}

void loop() {
  if (error == 1) { // skip loop if no controller found
    Serial.println("PS2 Controller error");
    return;
  }
  else { // DualShock Controller
    ps2x.read_gamepad(false, vibrate); // read controller and set 'vibrate' speed
    // Set motor 1 speed with stick
    speed1s = ps2x.Analog(PSS_LY);
    speed1s = constrain(speed1s,0,255);
    if(speed1s > 95 && speed1s < 155) {
      speed1s = 128;
    }
    /*else if(speed1s > 155) {
      speed1s = map(speed1s,155,255,155,200);
    }
    else if(speed1s < 95) {
      speed1s = map(speed1s,0,95,55,95);
    }*/
    Wire.beginTransmission(md22Address);
    Wire.write(motor1);
    Wire.write(speed1s);
    Wire.endTransmission();
    // Set motor 2 speed with stick
    speed2s = ps2x.Analog(PSS_RY);
    speed2s = constrain(speed2s,0,255);
    if(speed2s > 95 && speed2s < 155) {
      speed2s = 128;
    }
    /*else if(speed2s > 155) {
      speed2s = map(speed2s,155,255,155,200);
    }
    else if(speed2s < 95) {
      speed2s = map(speed2s,0,95,55,95);
    }*/
    Wire.beginTransmission(md22Address);
    Wire.write(motor2);
    Wire.write(speed2s);
    Wire.endTransmission();
    // Baloon
    if (ps2x.ButtonPressed(PSB_BLUE)) {
      if (baloon == 1) {
        digitalWrite(4, HIGH);
        baloon = 0;
        Serial.println("Baloon ON");
        }
      else if (baloon == 0) {
        digitalWrite(4, LOW);
        baloon = 1;
        Serial.println("Baloon OFF");
        }  
      }
    // Servo up
    if (ps2x.ButtonPressed(PSB_R1)) {
      if (servoR1 == 1) {
        servoPos0(0);
        servoR1 = 0;
      }
      else {
        servoPos1(0);
        servoR1 = 1;
      }  
    }
    // Servo down
    if (ps2x.ButtonPressed(PSB_L1)) {
      if (servoL1 == 1) {
        servoPos0(1);
        servoL1 = 0;
      }
      else {
        servoPos1(1);
        servoL1 = 1;
      }  
    } 
  }
  delay(50);
}

void servoPos0 (int numServo){
  servoSerial.write(0xFF);
  servoSerial.write(numServo);
  servoSerial.print(char(0));
  Serial.println("Servo Position 0");
}

void servoPos1 (int numServo){
  servoSerial.write(0xFF);
  servoSerial.write(numServo);
  servoSerial.print(char(254));
  Serial.println("Servo Position 1");
}

void getMd22Version(){
  // get and print software revision to serial
  Wire.beginTransmission(md22Address);
  Wire.write(softReg);
  Wire.endTransmission();
  Wire.requestFrom(md22Address, 1);
  while(Wire.available() < 1);
  int software = Wire.read();
  Serial.print("MD22 Motor Control   V:");
  Serial.println(software, DEC);
}

void setMd22Mode(){
  // set mode to 2 and sets acceleration
  Wire.beginTransmission(md22Address);
  Wire.write(accelReg);
  Wire.write(80);
  Wire.endTransmission();
}

