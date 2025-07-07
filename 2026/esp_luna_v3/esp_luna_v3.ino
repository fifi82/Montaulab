
 
#include <Arduino.h>     // every sketch needs this
#include <Wire.h>        // instantiate the Wire library
#include <TFLI2C.h>      // TFLuna-I2C Library v.0.2.0
#include <ESP32Servo.h>
#include <VL53L0X.h>

VL53L0X lidarD;
VL53L0X lidarG;

// les servomoteurs
Servo servo_direction;
Servo servo_vitesse;

int minUs = 1000;
int maxUs = 2000;
int pin_servo_direction = 16;
int pin_servo_vitesse = 15;



// lidar luna tf mini
TFLI2C tflI2C;
int16_t  tfDist;    // distance in centimeters
int16_t  tfAddr = TFL_DEF_ADR;  // use this default I2C address or
                                // set variable to your own value

unsigned long t0,t1,t_frein; // tempo

int direction = 90;
int vitesse = 90;
int frein = 0;

void setup()
{

  pinMode(18,OUTPUT); // inhibition lidar droit
  digitalWrite(18,0);

  Serial.begin( 115200);  // initialize serial port
  Wire.begin();           // initialize Wire library
     
  pinMode(23,INPUT_PULLUP);

  ESP32PWM::allocateTimer(0);
	ESP32PWM::allocateTimer(1);
	ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);


  servo_direction.attach(pin_servo_direction, minUs, maxUs);
	servo_vitesse.attach(pin_servo_vitesse, minUs, maxUs);


  lidarG.init(true);// initialisation
  delay(500);
  lidarG.setAddress((uint8_t)0x28);
  delay(500);
         
  lidarG.setTimeout(500);
  //lidarG.setDistanceMode(VL53L1X::Long); // distance de lecture Long, Medium, Short
 // lidarG.setMeasurementTimingBudget(50000);
  lidarG.startContinuous(20);
  
	Serial.println( "Start ...");   

  //pinMode(18,INPUT); // active le lidar droit et le configure
  digitalWrite(18,1);
  lidarD.setTimeout(500);           // timeout en cas de défaut du lidar
  lidarD.init(true);      // initialisation
  
  //lidarD.setDistanceMode(VL53L1X::Long); // distance de lecture Long, Medium, Short
  //lidarD.setMeasurementTimingBudget(50000);
  lidarD.startContinuous(20);
  

}

void loop(){
  t0 = millis();

  if (t0>t1){
    t1 = t0 + 10;
    tflI2C.getData( tfDist, tfAddr); // récupère la distance du luna au centre
   
   
    if (tfDist<10){
      if (frein == 0) {
        vitesse = 20; // point mort
        frein = 1;
        t_frein = t0+100;
      } else if (frein == 1 and t0 > t_frein){
        frein = 2;
        vitesse = 80;
        t_frein = t0+50;
      } else if (frein == 2 and t0 > t_frein){
        frein = 3;
        t_frein = t0+50;
        vitesse = 70;
      } else if (frein == 3 and t0 > t_frein){
        t_frein = t0+50;
        if (vitesse > 58) vitesse -=1;
      }

    } else {
      frein = 0;// pas de freinage
      vitesse = 90 + tfDist/4;
      if (vitesse>150) vitesse = 150;
    } 
    
    int ld = lidarD.readRangeSingleMillimeters();
    int lg = lidarG.readRangeSingleMillimeters();

    direction = 90+(ld - lg );
    if (direction<20)direction=20;
    else if (direction>170)direction=170;
    if (digitalRead(23))  vitesse = 80;
    //Serial.println("distance = " + String(tfDist) + ", Vitesse = " + String(vitesse) + ", ld = " + String(ld) +  ", lg = " + String(lg));
    Serial.println("direction = " + String(direction) + ", ld = " + String(ld) +  ", lg = " + String(lg));
    
    servo_vitesse.write(vitesse);
    servo_direction.write(direction);

  } // if (t0>t1)



   
 
} // loop 

