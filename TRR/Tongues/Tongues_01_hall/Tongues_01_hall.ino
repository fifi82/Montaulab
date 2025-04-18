#include <Servo.h>


Servo servo;

const int droite=30;  // angle pour tourner à gauche
const int centre=90;  // angle pour tourner au centre
const int gauche=130; // angle pour tourner à droite
int v_servo=centre;   // valeur courante du servo
int _dir=1; // 0=gauche, 1=centre, 2=droite

const int b_hall=12;   // broche du transistor effet hall 
const int b_hall_p=10;   // broche du transistor effet hall +5v
const int b_hall_m=11;   // broche du transistor effet hall 0v 
bool m_hall;          // mémoire état du transistor à effet hall
unsigned long t0,t1,t2; //temps entre de pulse du capteur hall pour en deduire le 1/2 tour
bool anti_r=false;

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
    
    pinMode(b_hall,INPUT_PULLUP);   // capteur hall
    pinMode(b_hall_p,OUTPUT);   // capteur hall +5v
    digitalWrite(b_hall_p,HIGH);// capteur hall +5v
    pinMode(b_hall_m,OUTPUT);   // capteur hall 0v
    
    pinMode(b_mot,OUTPUT);      // moteur
    pinMode(13,OUTPUT);      // led
 // Serial.begin(9600);
}

void loop() {
  
  // lecture télécommande
  bool bp_A=digitalRead(b_bp_A);
  bool bp_B=digitalRead(b_bp_B);
  bool bp_C=digitalRead(b_bp_C);
  bool bp_D=digitalRead(b_bp_D);
  bool hall=!digitalRead(b_hall);
  
  if (bp_A) {digitalWrite(  // si appuis sur bouton A marche au centre
    b_mot,HIGH); 
    _dir=1;
  } 
  else if (bp_B) digitalWrite(b_mot,LOW);   // si appuis sur bouton B = arret
  else if (bp_C) _dir=0; 
  else if (bp_D) _dir=2;
     
  if (hall and hall!=m_hall) {
    
    t1=millis()-t0; // calcule le temps pour faire un tour
    t0=millis();    // initialise la tempo
    t2=t0+t1/2;     // calcule le temps pour faire un 1/2 tour
    if (_dir==1 and v_servo!=centre) v_servo=centre; // si on va au centre ...
    else if (_dir==0) v_servo=droite; // tourne à droite 
    else if (_dir==2) v_servo=gauche; // tourne à gauche
    servo.write(v_servo);             // fait bouger le servo
    anti_r=true;                      // anti rebond
  } else {
    
    if (millis()>t2 and anti_r==true ){  // si 1/2 tour est éffectué
      anti_r=false;                     // anti rebond
      if (_dir==0) v_servo=gauche;      // tourne à gauche
      else if (_dir==2) v_servo=droite; // tourne à droite 
      servo.write(v_servo);             // fait bouger le servo
    }
  }
  if (hall) digitalWrite(13,HIGH); else digitalWrite(13,LOW);
  m_hall=hall;
}
