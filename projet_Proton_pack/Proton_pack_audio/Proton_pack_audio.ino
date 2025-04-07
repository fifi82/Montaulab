#include <Adafruit_NeoPixel.h>

#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

SoftwareSerial mySoftwareSerial(10, 11); // RX, TX
DFRobotDFPlayerMini myDFPlayer;

#define t0 millis()

#define nb_led 15 // nonbre de led 0 à 3 les 4 leds du backpack, 4 à 14 état de charge du Proton_Pack
#define pin_led 3 // broche data des leds néopixel ws2812
#define pin_bp1 4 // bouton tir
#define pin_masse 5 // masse du bouton

Adafruit_NeoPixel pixels(nb_led, pin_led, NEO_GRB + NEO_KHZ800);


int mled_4, led_4; // variables pour les 4 leds du backpack
int v1 = 500; // vitesse de défilement des 4 leds du backpack

int Proton_Pack = 0; // état du Proton_Pack, 0=attente, 1=chargement,2=attente de tir, 3=tir en cour
int pos;  // position de la led à allumer ou à éteindre du Proton_Pack 
int v2 = 150; // vitesse de défilement pendant le chargement du Proton_Pack 
int v3 = 10;  // vitesse de défilement pendant le tir Proton_Pack
int intencite;
int i_max = 100;
int dir = 1;

unsigned long t1,t2; // tempo

void setup() {
  Serial.begin(9600);
  
  mySoftwareSerial.begin(9600); // port serie du DFPlayer
 
   if (!myDFPlayer.begin(mySoftwareSerial)) {  //erreur
    Serial.println(F("!!!!"));
    Serial.println(F("1.Erreur verifiez la connectique!"));
    Serial.println(F("2.Inserez une carte SD!"));
    while(true);
  }
  Serial.println(F("DFPlayer Ok."));

  myDFPlayer.volume(30);  //Set volume value. From 0 to 30
  
  myDFPlayer.play(1);  // fichier à lire -1

  pinMode(pin_bp1,INPUT_PULLUP); // bouton armement et tir
  
  pixels.begin(); 
   
  pixels.clear(); // Set all pixel colors to 'off'
  pixels.show();   // Send the updated pixel colors to the hardware.

  pinMode(pin_bp1,INPUT_PULLUP);
  pinMode(pin_masse,OUTPUT); digitalWrite(pin_masse,0);
}

void loop() {
    
  ////////////////////////////////////  animation des 4 leds du backpack  //////////////////////////////
  led_4 = (t0 / v1)%4; // calcul de la led à allumer (0,1,2 ou 3 en fonction du temps)
  if (led_4 != mled_4){ // si une nouvelle led doit être allumée (toute les v1 (500ms)) 
    pixels.setPixelColor(mled_4 , 0x000000); // eteint la led mémorisé dans mled_4
    pixels.setPixelColor(led_4 , 0xff0000); // allume une des 4 led 
    mled_4 = led_4;  // mémorise la led allumée pour l'éteindre par la suite   
  }

  /////////////////////////////////////  animation du Proton_Pack (fusil) ///////////////////////////////////////
  if (Proton_Pack == 0){ // attente de chargement
    if (!digitalRead(pin_bp1) ){ // si appuis sur le  bouton
    myDFPlayer.play(2);
    Proton_Pack = 1; // chargement en cours
    pos = 4; // 1ere led à allumer
    t2 = t0 + 60000; // au bout de une minute on désactive le laser
    }
  }
  else if (Proton_Pack == 1){  // recharge en cours   
   pinMode(2,INPUT); 
    if (  t0 > t1 ){ // si la tempo t1 est finie
      t1 = t0 + v2; // réarme la tempo à V2 (200ms)
      pixels.setPixelColor(pos ,0,i_max,0 );
      pos++;      // led suivante
      if( pos > nb_led ) {// si toute les led sont allumée => attente de tir
        Proton_Pack = 2;
        intencite = i_max;

      }
    }
  }
  else if (Proton_Pack == 2){// attente de tir
    if ( !digitalRead(pin_bp1) ){  // si appuis sur le bouton 
      Proton_Pack = 3; // tir en cours 
      pos = 4;// 1ere led à éteindre
      myDFPlayer.play(3);
    }
    if (t0>t1){ // atente tempo 1
      t1 = t0 + 10; // init tempo 1 à 10ms
      if (intencite <20 or intencite > i_max) dir = -dir; // on inverse la variation
      for (int i=4; i<nb_led; i++) pixels.setPixelColor(i , 0 ,intencite, 0 ); // ajuste la couleur de toutes les leds
      intencite += dir; // modifi l'intencité
    }
    if (t0>t2) { // fin du fichier son
      Proton_Pack = 0;
      for (int i=4; i<nb_led; i++) pixels.setPixelColor(i , 0 ); // 
      }
  }
  else if (Proton_Pack == 3){// tir en cours 
    if ( t0 > t1 ){ // si la tempo t1 est finie
      t1 = t0 + v3*3; // réarme la tempo à V3 (10ms)
      pixels.setPixelColor(pos , 0xff0000 ); // éteind la led 
      pos++; // led suivante 
      if(pos == nb_led) {// si toutes les leds sont éteintes => attente de chargement
        Proton_Pack = 4;
        pos = 4;
      } 
    }
  } 
  else if (Proton_Pack == 4){// tir en cours 
    if ( t0 > t1 ){ // si la tempo t1 est finie
      t1 = t0 + v3; // réarme la tempo à V3 (10ms)
      pixels.setPixelColor(pos , 0 ); // éteind la led 
      pixels.setPixelColor(pos + 1, 0xffffff );// allume en blanc la led suivante
      pos++; // led suivante 
      if(pos == nb_led+1) Proton_Pack = 0; // si toutes les leds sont éteintes => attente de chargement
    }
  } 
  
  pixels.show(); // raffraichit le ruban de led


}










