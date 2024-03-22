// #include <Config.h>
// #include <Firebase.h>
// #include <Firebase_Arduino_WiFi101.h>
// #include <Firebase_TCP_Client.h>
// #include <WCS.h>

#include<LiquidCrystal.h>
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <EEPROM.h>
#include <Stepper.h>
#include <SoftwareSerial.h>
#include <DHT.h>
// #include <FirebaseArduino.h>

#define FIREBASE_HOST "https://electronics-34416-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "AIzaSyDsRg_5R4hQkIfJXE6d23xFOblzE4dMw4U"

// SoftwareSerial BTSerial(0, 1); // RX, TX pins for Bluetooth communication
SoftwareSerial comSerial(19,18);
const int stepsPerRevolution = 200;  // change this to fit the number of steps per revolution
// for your motor
#define DHTPIN 29    // Pin for the DHT sensor
#define DHTTYPE DHT11 // DHT 22 (AM2302)

DHT dht(DHTPIN, DHTTYPE);
float motorTemperatureThreshold = 40.0;


// initialize the stepper library on pins 8 through 11:
Stepper myStepper(stepsPerRevolution, 25, 26, 27, 28);

int stepCount = 0; 
LiquidCrystal lcd(2,3,4,5,6,7);

#define echopin A4 // echo pin
#define trigpin A5 // Trigger pin
int PotPin = A7; 
int MosfetPin = 22;

int cont = 0;
int st=0;

int m1=52, m2=51, en=50;

int in1 = A0;
int in2 = A1;
int in3 = A2;
int in4 = A3;

int remotePins [] = { A0, A1, A2, A3};

int set = 0;
int stop = 0, stop1;
int mode = 0;

int buz = 13;

int timer = 0;
 
int pump = 8;
int motor = 9;

int ir_start = 10;
int ir_fill = 11;
int ir_stop = 12;
int temperature;
int percentage;
int bottles = 0;
int dist;
int MAX_DISTANCE = 1000;

long duration;

int val1=0,val2=0,val3=0,val4=1;
long filltimer=1000;

void setup(){
   dht.begin();
  comSerial.begin(9600);

   
    
 for (int i = 0; i < 5; i ++) {
  pinMode (remotePins [i], INPUT);
  digitalWrite(remotePins [i], HIGH);
}

    pinMode(ir_fill, INPUT);
    pinMode(ir_start, INPUT);
    pinMode(ir_stop, INPUT);

    pinMode (trigpin, OUTPUT);
    pinMode (echopin, INPUT);

    pinMode(motor, OUTPUT);
    pinMode(pump, OUTPUT);

    pinMode(buz, OUTPUT);
    pinMode(m1, OUTPUT);
    pinMode(m2, OUTPUT);
    pinMode(en, OUTPUT);
    pinMode(DHTPIN, INPUT);
    // pinMode(MosfetPin, OUTPUT); 

    lcd.begin(16,4);
    lcd.setCursor(0,0);lcd.print("   WELCOME  To  ");
    lcd.setCursor(0,1);lcd.print("Motor Control");
    lcd.setCursor(0,2);lcd.print("  Dosing");
    lcd.setCursor(0,3);lcd.print("          System");
    delay(500); 
    //Write();
    Read();
    //analogWrite(motor, 100);
    for (int i = 0; i < 5; i ++) {
      dist = data1();
      // data2();
      delay(100); 
    }
    lcd.clear(); 
}

void loop(){
  

    int device_id = 1;
// function to set timer
  if(stop==0){
    if(digitalRead (in1) == 0){
    lcd.clear();
    mode = mode+1; 
    if(mode>4){
      mode=0;
    Write();
    Read();
    lcd.setCursor(0,0);
    lcd.print("  Ok Saved Data ");
    delay(200);
    }
  delay(300);
  }

    if(digitalRead (in2) == 0){
    if(mode==1){val1 = val1+1;}
    if(mode==2){val2 = val2+1;}
    if(mode==3){val3 = val3+1;}
    if(mode==4){val4 = val4+1;}
    if(val1>9){val1=0;}
    if(val2>9){val2=0;}
    if(val3>9){val3=0;}
    if(val4>9){val4=0;}
    delay(300);
    }
}

  if(mode==0){
  if(digitalRead (in3) == 0){stop=0;}
  if(digitalRead (in4) == 0){stop=1;}

    lcd.setCursor(0,0);lcd.print("WELCOME To MCDS");
    // lcd.setCursor(0,1);lcd.print("     M C D S    ");


    lcd.setCursor(0,1);
    lcd.print("fillLevel = ");
    // comSerial.println(bottles);
    // lcd.print(percentage);
    lcd.print(dist);
    // lcd.setCursor(0,2);
    // lcd.print("Tank Level=");
    // // lcd.print(percentage);
    // // lcd.print(dist);
    // lcd.print(dist);

    // lcd.print();

    lcd.print(" ");

    lcd.setCursor(0,3);
    lcd.print("Fill Time =");
    lcd.print(filltimer);  
    // comSerial.println(filltimer);

    lcd.print("   ");
    }else{

      lcd.setCursor(0,0);
      lcd.print("Set Time= ");
      lcd.print(val4); 
      lcd.print(val3);
      lcd.print(val2);
      lcd.print(val1);

      if(mode==4) {lcd.setCursor(10,1);lcd.print("-");} 
      if(mode==3) {lcd.setCursor(11,1);lcd.print("-");}
      if(mode==2) {lcd.setCursor(12,1);lcd.print("-");} 
      if(mode==1) {lcd.setCursor(13,1);lcd.print("-");} 
    }



// this function initializes the process of the two motors{pump and conveyor belt}
  if (stop == 1) {
  if (digitalRead(ir_stop) == 1) {
     digitalWrite(buz, LOW);
      

      //STEPPER
      int sensorReading = analogRead(A7);
      // map it to a range from 0 to 100:
      int motorSpeed = map(sensorReading, 0, 1023, 0, 100);
      // set the motor speed:
      lcd.setCursor(0,2);
      lcd.print("Speed= ");
      // lcd.print(percentage);
      // lcd.print(dist);
     

      delay(500); // Adjust as needed
      lcd.print(motorSpeed);
      if (motorSpeed > 0) {
      myStepper.setSpeed(motorSpeed);
      // step 1/100 of a revolution:
      myStepper.step(stepsPerRevolution / 100);
      }
    lcd.setCursor(0,2);
    lcd.print("Speed =");
    lcd.print(motorSpeed);  
    comSerial.print(motorSpeed);
     comSerial.print(";");

      
    if (digitalRead(ir_fill) == 0) {
      if (stop1 == 0) {
        stop1 = 1;
        // analogWrite(motor, 0);
        // analogWrite(en, 0);
        // myStepper.setSpeed(0);

        delay(100);
        digitalWrite(pump, HIGH);
        delay(filltimer);
        bottles +=1;
        digitalWrite(pump, LOW);

        // Measure and display distance after filling
        for (int i = 0; i < 10; i++) {
          dist = data1();
          // data2();
          delay(200);
        }
       
     
        analogWrite(motor, 200);
        
        
      }
    }

    // digitalWrite(buz, HIGH);

    if (digitalRead(ir_start) == 0) {
      stop1 = 0;
    }
  } else {
    analogWrite(motor, 0);

    // myStepper.setSpeed(0);

    digitalWrite(buz, HIGH);

    delay(300);
  }
} else {
  analogWrite(motor, 0);
  // myStepper.setSpeed(0);
      //
      
     
} 

  comSerial.print(bottles);
  comSerial.print(";");
  comSerial.print(temperature);
  comSerial.print(";");

  // comSerial.print(filltimer);
  // comSerial.print(";");
    // Serial.print(";");
  comSerial.println(device_id);
  delay(500);
  // Read temperature and humidity
    temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    // Check if the temperature exceeds the threshold
    // if (temperature > motorTemperatureThreshold)
    // {
    //     // Perform actions for high temperature
    //     // digitalWrite(buz,HIGH);
    //     stop = 0;
    // }else{
    //   // stop=1;
    //     // digitalWrite(buz,LOW);

    // }

}

void Read(){
val1 = EEPROM.read(11); val2 = EEPROM.read(12); val3 = EEPROM.read(13); val4 = EEPROM.read(14);  
filltimer = val4*1000+val3*100+val2*10+val1;  
Serial.println(filltimer);  
}

void Write(){  
EEPROM.write(11, val1);EEPROM.write(12, val2);EEPROM.write(13, val3);EEPROM.write(14, val4);
}

long data1()
{
  long duration, distance;

digitalWrite(trigpin, LOW);
delayMicroseconds(2);
digitalWrite(trigpin, HIGH);
delayMicroseconds(10);
digitalWrite(trigpin, LOW);
duration = pulseIn(echopin, HIGH);
dist = (duration / 2) / 29.1;

// Map the distance to a percentage (adjust the range based on your specific requirements)
int percentage = map(dist, 0, MAX_DISTANCE, 0, 100);

// Ensure the percentage is within the valid range
percentage = constrain(percentage, 0, 100);
}