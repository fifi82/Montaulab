
 
#include <Arduino.h>     // every sketch needs this
#include <Wire.h>        // instantiate the Wire library
#include <TFLI2C.h>      // TFLuna-I2C Library v.0.2.0
#include <ESP32Servo.h>

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
  Serial.begin( 115200);  // initialize serial port
  Wire.begin();           // initialize Wire library
     
  pinMode(23,INPUT_PULLUP);

  ESP32PWM::allocateTimer(0);
	ESP32PWM::allocateTimer(1);
	ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);


  servo_direction.attach(pin_servo_direction, minUs, maxUs);
	servo_vitesse.attach(pin_servo_vitesse, minUs, maxUs);

	Serial.println( "Start ...");   
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


    Serial.println("distance = " + String(tfDist) + ", Vitesse = " + String(vitesse));
    if (digitalRead(23))  vitesse = 80;
    servo_vitesse.write(vitesse);
    servo_direction.write(direction);

  } // if (t0>t1)



   
 
} // loop 

