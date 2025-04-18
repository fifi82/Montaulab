#include <Servo.h>

Servo servo;


// Télécommande
const int b_bp_A=3; // broche bouton A
const int b_bp_B=4; // broche bouton B
const int b_bp_C=6; // broche bouton C
const int b_bp_D=5; // broche bouton D

//moteur D9
const int b_mot=9;


void setup() {
  servo.attach(8); //,544,2400);
  
    // config télécommande
    pinMode(2,OUTPUT);digitalWrite(2,HIGH); // +5v
    pinMode(b_bp_A,INPUT);   // boutonA
    pinMode(b_bp_B,INPUT);   // boutonB
    pinMode(b_bp_C,INPUT);   // boutonC
    pinMode(b_bp_D,INPUT);   // boutonD

    pinMode(b_mot,OUTPUT);
  
 // Serial.begin(9600);
}

void loop() {

  // lecture télécommande
  bool bp_A=digitalRead(b_bp_A);
  bool bp_B=digitalRead(b_bp_B);
  bool bp_C=digitalRead(b_bp_C);
  bool bp_D=digitalRead(b_bp_D);
  
  if (bp_A) digitalWrite(b_mot,HIGH);  // marche
  if (bp_B) digitalWrite(b_mot,LOW);   // arret
  
  if (bp_C) servo.write(10);
  if (bp_D) servo.write(150);  
    
  //Serial.print(bp_A);  Serial.println();

  
}
