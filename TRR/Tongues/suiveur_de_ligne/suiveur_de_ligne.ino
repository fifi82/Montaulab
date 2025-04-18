

#include <Servo.h>

Servo Servo_direction;


const int b_r[]= {A1,A2,A3,A4,A5,A6,A0}; // broche des récepteurs

const int b_e[]={2,3,4,5,6,7,8};         // broche des led IR éméteur

int lec[7];
int moy,moyenne,dir; // moy = l'ensemble des capteurs moyenne c'est la moyenne et dir la valeur du servo en degré de 0 à 180
int v_dir=90;

void setup() {

  Servo_direction.attach(9); 
  
  for ( int i=0; i<7; i++) { 
    pinMode(b_e[i],OUTPUT);
    pinMode(b_r[i],INPUT);   digitalWrite(b_r[i],HIGH); // pull up
  }
  
  //Serial.begin(115200);
}

void loop() {

  for ( int i=0; i<7; i++){ // passe en revue les 7 capteurs
    
    digitalWrite(b_e[i],HIGH); // allume un capteur pour faire une lecture

    delay(3);
    
    moy -= lec[i]; // enlève une lecture d'un des capteurs
    lec[i]=analogRead(b_r[i]);  // lecture du capteur
    
    digitalWrite(b_e[i],LOW);
    
    moy += lec[i]; // remplace cette lecture par une nouvelle
    
    moyenne=(moy/7)-20;
    
    if (lec[i]>moyenne) dir=map(i, 0, 6, 45, 135) ;

    if (dir>v_dir)v_dir++;
    if (dir<v_dir)v_dir--; 
    
    Servo_direction.write(v_dir);

  // aff(i);
  }

}

void aff(int j){

   Serial.print("A");Serial.print(j+1);Serial.print("=");
   Serial.print(lec[j]); Serial.print(" / ");
  if(j==6){
   Serial.print(" / ");  
   Serial.print("v_dir=");
   Serial.print(v_dir);
   
   Serial.print(" / ");  
   Serial.print("moyenne=");
   Serial.print(moyenne);
  

  Serial.println();
  delay(100); 
  }
}

