#include <Adafruit_NeoPixel.h>  // https://github.com/adafruit/Adafruit_NeoPixel_ZeroDMA
#include <LiquidCrystal_I2C.h>  // https://github.com/johnrickman/LiquidCrystal_I2C

LiquidCrystal_I2C lcd(0x27,16,2); // écran LCD 16x2

#define nb_led 22
Adafruit_NeoPixel pixels(nb_led, 6, NEO_GRB + NEO_KHZ800); // ruban de 22 leds WS2812b

// pour simplifier le code ...
#define v1_on digitalWrite(pin_v1, HIGH)
#define v2_on digitalWrite(pin_v2, HIGH)
#define v3_on digitalWrite(pin_v3, HIGH)
#define v1_off digitalWrite(pin_v1, LOW)
#define v2_off digitalWrite(pin_v2, LOW)
#define v3_off digitalWrite(pin_v3, LOW)
#define v1 digitalRead(pin_v1)
#define v2 digitalRead(pin_v2)
#define v3 digitalRead(pin_v3)
#define bp1 !digitalRead(pin_bp1)
#define bp2 !digitalRead(pin_bp2)
#define bp3 !digitalRead(pin_bp3)
#define hall1 !digitalRead(pin_bp3)


#define pin_roue1 2 // pin du capteur effet hall roue 1
#define pin_roue2 3 // pin du capteur effet hall roue 2
#define roue1 !digitalRead(pin_roue1)
#define roue2 !digitalRead(pin_roue2)

#define pin_bp1 10 // pin du bouton 1 coté roue gauche
#define pin_bp2 8  // pin du bouton 2 coté roue droite
#define pin_bp3 12 // pin du bouton 3 au centre

#define pin_v1 11 // pin du voyant bouton 1 coté roue gauche
#define pin_v2 9  // pin du voyant bouton 2 coté roue droite
#define pin_v3 13 // pin du voyant bouton 3 au centre

volatile unsigned long t1 = 99999,   // temps de rotation roue 1
                       t2 = 99999,   // temps de rotation roue 2
                       mt1 = 99999,  // mémoire temps de rotation roue 1
                       mt2 = 99999,  // mémoire temps de rotation roue 2
                       max1 = 99999, // temps maxi roue 1
                       max2 = 99999, // temps maxi roue 2
                       record = 99999;
                       

                       
unsigned long t0,  // temps de refférence,
               tv1,// tempo voyant 1
               tv2,// tempo voyant 2
               tv3,// tempo voyant 3
               tbp,// tempo anti rebond des boutons poussoir
               t3, // tempo 3
               tl1; // tempo 1 leds
               

volatile byte etape = 0;  // état du jeu, 0=attente, 1=sélection du jeu,
byte jeu = 0;  // état dans un jeu

int menu;
byte nb_menu = 4; 
String t_menu[] = {"battle", "Speed Solo roue1", "Speed Solo roue2", "Voir le record"};
String  texte;
int len,pos; // longeur du texte et position du texte

int mpos_led, pos_led, dir_led=1;

/********************  setup *******************/
void setup() {

  attachInterrupt(digitalPinToInterrupt(pin_roue1), top_roue1, RISING);
  attachInterrupt(digitalPinToInterrupt(pin_roue2), top_roue2, RISING);

  Serial.begin(9600);

  pinMode( pin_roue1 , INPUT_PULLUP); // entrée du capteur effet hall roue 1
  pinMode( pin_roue2 , INPUT_PULLUP); // entrée du capteur effet hall roue 2
  
  pinMode( pin_bp1 , INPUT_PULLUP); // entrée du bouton 1
  pinMode( pin_bp2 , INPUT_PULLUP); // entrée du bouton 2
  pinMode( pin_bp3 , INPUT_PULLUP); // entrée du bouton 3

  pinMode( pin_v1 , OUTPUT); // sortie voyant du bouton 1
  pinMode( pin_v2 , OUTPUT); // sortie voyant du bouton 2
  pinMode( pin_v3 , OUTPUT); // sortie voyant du bouton 3

  lcd.init();
  lcd.backlight();

  pixels.begin();  
  pixels.clear();
  pixels.show();

}

/********************  top_roue1 *******************/
void top_roue1(){
  t1 =  millis() - mt1;
  mt1 = millis();
  if(etape>1){
    if (t1 < max1) max1 = t1;
    if (t1 < record) record = t1;    
  }

}


/********************  top_roue2 *******************/
void top_roue2(){
  t2 = millis() - mt2;
  mt2 = millis();
  if(etape>1){
    if (t2 < max2) max2 = t2;
    if (t2 < record) record = t2;    
  }
}


/********************  loop *******************/
void loop() {
  t0 = millis(); // mémorise le temps de référence

  if      (etape == 0) etape0();
  else if (etape == 1) etape1();
  else if (etape == 2) etape2();
  else if (etape == 3) etape3();
  else if (etape == 4) etape4();  

}


/********************  etape0 *******************/
void etape0(){ // attente

  anim1();

  clignotte(3); // faire clignotter le bouton 3

  if (bp3) { // selection du jeu si bouton 3
    etape = 1; // étape du jeu 
    tbp = t0 + 300; // anti rebond du bouton poussoir
    pixels.clear();
    pixels.show();
  } 



  lcd.setCursor(0,0);
  lcd.print("Record " + String(int( 1000.0/record*400 ) )+ " mm/s   ") ;
    
  lcd.setCursor(0,1);
  lcd.print( "R1:" +  String(int( 1000.0/t1*400 ) ) + " R2:" +  String(int( 1000.0/t2*400 ) ) + "      "); // un tour de roue = 400 mm
  /*
  lcd.print("r2 = ");
  lcd.print( int( 1000.0/t2*400 ) );
  lcd.print(" mm/s            ");
*/
}

/********************  etape1 *******************/
void etape1(){ // menu du jeu

  clignotte(1); 
  clignotte(2); 
  clignotte(3);

  if (bp1 and t0 > tbp){ // bouton 1 menu +
    tbp = t0 + 300; // anti rebond
    menu ++;       // change le menu
    if (menu == nb_menu) menu = 0; // si fin de menu on recommance
  } else if (bp2 and t0 > tbp){ // bouton 3 valide
    tbp = t0 + 300; // anti rebond    
    menu --;
    if (menu < 0) menu = nb_menu - 1; // si fin de menu on recommance
  } else if (bp3 and t0 > tbp){ // bouton 3 valide
    v1_off;
    v2_off;
    v3_off;
    if (menu == 0) {  // battle
      etape = 2; 
      texte = "Tourner les roues pour allumer les leds  ";
      init_jeu();          
    }
    else if (menu == 1) { // Speed solo roue1 
      etape = 3; 
      texte = "Tourner la roue 1 pour allumer la led ";
      init_jeu();
    }
    else if (menu == 2) {// Speed solo roue2  
      etape = 4; 
      texte = "Tourner la roue 2 pour allumer la led ";
      init_jeu();
    }   
    else if (menu == 3) etape = 0;      // pas utilisé encore
  }

  lcd.setCursor(0,0);
  lcd.print("Choix du jeu : ");

  lcd.setCursor(0,1);
  lcd.print(t_menu[menu] + "          ");
}


/********************  etape2 *******************/
void etape2(){ // battle
  if(jeu==0) { // placement des roues
    lcd.setCursor(0,0);
    lcd.print("Battle R1 / R2  ");      
    lcd.setCursor(0,1);
    texte_long();  
    if(roue1) v1_on; else v1_off;
    if(roue2) v2_on; else v2_off;

    if(roue1 and roue2) {
      jeu=1; 
      t3 = t0 + 10000;
      v1_on;
    }     
  }
  else if (jeu==1){ // décompte avent début du jeu
    lcd.setCursor(0,0);  
    lcd.print("Pret a tourner  "); 
    lcd.setCursor(0,1);  
    lcd.print("dans : " + String(1 + (t3 - t0)/1000 ) + "                     ");   
    if (t0>t3) { // start du jeu 1
      jeu=2; 
      t3 = t0 + 5000;  
      max1 = 99999; // temps pour une rotation
    }
    if(!roue1 or !roue2) {
      jeu = 0;
      t3 = t0 + 2000;
      v1_off;
      v2_off;
      pos = 0;
    }
  } 
  else if (jeu==2){ // décompte avent début du jeu
    lcd.setCursor(0,0);
    lcd.print( "R1:" +  String(int( 1000.0/t1*400 ) ) + " R2:" +  String(int( 1000.0/t2*400 ) ) + "      "); // un tour de roue = 400 mm

    lcd.setCursor(0,1);  
    lcd.print("Tournez ... " + String(1 + (t3 - t0)/1000 ) + "        ");
    if (t0>t3){
      jeu = 3; // résultat
      t3 = t0 + 10000;          
    }
  }
  else if (jeu==3){ // résultat
    //  lcd.print( int( 1000.0/t1*400 ) ); // un tour de roue = 400 mm
    lcd.setCursor(0,0);
    lcd.print("record " + String(int( 1000.0/record*400 )) + " mm/s         ");
    lcd.setCursor(0,1);
    lcd.print("R1:" + String(int( 1000.0/max1*400 )) + " R2:" + String(int( 1000.0/max2*400 )) + "   "); 
    if(t0>t3) {
      etape = 0;
      pos=0;
    }
  }

 

} // fin étape 2


/********************  etape3 *******************/
void etape3(){ // Speed solo roue1  

  if(jeu==0) { // placement de la roue 1
    lcd.setCursor(0,0);
    lcd.print("Speed solo roue1 ");      
    lcd.setCursor(0,1);
    texte_long();    
    if(roue1) {
      jeu=1; 
      t3 = t0 + 10000;
      v1_on;
    }     
  } 
  else if (jeu==1){ // décompte avent début du jeu
    lcd.setCursor(0,0);  
    lcd.print("Pret a tourner  "); 
    lcd.setCursor(0,1);  
    lcd.print("dans : " + String(1 + (t3 - t0)/1000 ) + "                     ");   
    if (t0>t3) { // start du jeu 1
      jeu=2; 
      t3 = t0 + 5000;  
      max1 = 99999; // temps pour une rotation
    }
    if(!roue1) {
      jeu = 0;
      t3 = t0 + 2000;
      v1_off;
      pos = 0;
    }
  }
  else if (jeu==2){ // décompte avant début du jeu
    lcd.setCursor(0,0);
    lcd.print("r1 = ");
    lcd.print( int( 1000.0/t1*400 ) ); // un tour de roue = 400 mm
    lcd.print(" mm/s         ");
    lcd.setCursor(0,1);  
    lcd.print("Tournez ... " +  String(1 + (t3 - t0)/1000 )+ "       ");
    if (t0>t3){
      jeu = 3;
      t3 = t0 + 10000;          
    }
  }
  else if (jeu==3){ // résultat
    lcd.setCursor(0,0);
    lcd.print("record " + String(int( 1000.0/record*400 )) + " mm/s         ");
    lcd.setCursor(0,1);
    lcd.print("vous = " + String(int( 1000.0/max1*400 )) + " mm/s         "); 
    if(t0>t3) {
      v1_off;
      etape = 0;
    }   
  }
// Serial.println(jeu); 
  
}


/********************  etape4 *******************/
void etape4(){ // Speed solo roue2 

  if(jeu==0) { // placement de la roue 2
    lcd.setCursor(0,0);
    lcd.print("Speed solo roue2 ");      
    lcd.setCursor(0,1);
    texte_long();    
    if(roue2) {
      jeu=1; 
      t3 = t0 + 10000;
      v2_on;
    }     
  } 
  else if (jeu==1){ // décompte avent début du jeu
    lcd.setCursor(0,0);  
    lcd.print("Pret a tourner  "); 
    lcd.setCursor(0,1);  
    lcd.print("dans : " + String(1 + (t3 - t0)/1000 ) + "                     ");   
    if (t0>t3) { // start du jeu 1
      jeu=2; 
      t3 = t0 + 5000;  
      max2 = 99999; // temps pour une rotation
    }
    if(!roue2) {
      jeu = 0;
      t3 = t0 + 2000;
      v2_off;
      pos = 0;
    }
  }
  else if (jeu==2){ // décompte avent début du jeu
    lcd.setCursor(0,0);
    lcd.print("r2 = ");
    lcd.print( int( 1000.0/t2*400 ) ); // un tour de roue = 400 mm
    lcd.print(" mm/s         ");
    lcd.setCursor(0,1);  
    lcd.print("Tournez ... " +  String(1 + (t3 - t0)/1000 )+ "       ");
    if (t0>t3){
      jeu = 3;
      t3 = t0 + 10000;          
    }
  }
  else if (jeu==3){ // décompte avent début du jeu
    lcd.setCursor(0,0);
    lcd.print("record " + String(int( 1000.0/record*400 )) + " mm/s         ");
    lcd.setCursor(0,1);
    lcd.print("vous = " + String(int( 1000.0/max2*400 )) + " mm/s         "); 
    if(t0>t3) {
      etape = 0;
    }   
  }
  
}


/********************  etape5 *******************/
void etape5(){ //pas défini encore

}


/******************** clignotte  *******************/
void clignotte(int v){ // fait clignotter les boutons
  if(v==1){
    if(t0>tv1){
      tv1 = t0 + 100;
      if(v1) v1_off; else v1_on;
    }
  } else if(v==2){
    if(t0>tv2){
      tv2 = t0 + 100;
      if(v2) v2_off; else v2_on;
    }
  } else if(v==3){
    if(t0>tv3){
      tv3 = t0 + 100;
      if(v3) v3_off; else v3_on;
    }
  }
}


/********************  texte_long *******************/
void texte_long(){ // fait clignoter les leds des boutons
  lcd.print( texte.substring(pos, 16+pos) ); 
  if (t0>t3){
    t3=t0+300;
    pos++;
    if (pos > len+16){
      pos = 0;
      t3=t0+2000;
    }
  }
}


/******************** init_jeu  *******************/
void init_jeu(){ // reset des variables
      len = texte.length()-16;
      pos=0;
      t3 = t0 + 2000;
      jeu = 0;
}

/******************** anim1  *******************/
void anim1(){ // animation des néopixels
  if (t0>tl1){
    tl1 = t0 + 10;
    pos_led += dir_led;
    if (pos_led > nb_led-2 ) dir_led = -1;
    else if(pos_led<1) dir_led = 1;
    pixels.setPixelColor(mpos_led , 0 );
    pixels.setPixelColor(pos_led ,0xff0000  );
    mpos_led = pos_led;
    pixels.show();
  }
}



