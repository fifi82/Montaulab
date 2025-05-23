#include <Adafruit_NeoPixel.h>  // https://github.com/adafruit/Adafruit_NeoPixel_ZeroDMA
#include <LiquidCrystal_I2C.h>  // https://github.com/johnrickman/LiquidCrystal_I2C
#include <EEPROM.h>

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
#define bp1 !digitalRead(pin_bp1) and t0 > tbp
#define bp2 !digitalRead(pin_bp2) and t0 > tbp
#define bp3 !digitalRead(pin_bp3) and t0 > tbp
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
                       record;       // temps record (stocké en EEprom)
                       
                       
unsigned long t0,  // temps de refférence,
               tv1,// tempo voyant 1
               tv2,// tempo voyant 2
               tv3,// tempo voyant 3
               tbp,// tempo anti rebond des boutons poussoir
               t3, // tempo 3
               tt, // tempo texte long
               tl1; // tempo 1 leds
               

volatile byte etape = 0,  // état du jeu, 0=attente, 1=sélection du jeu,
          nbt1, // calcul du nonbre de tour de la roue 1
          nbt2; // calcul du nonbre de tour de la roue 2
byte jeu = 0;   // état dans un jeu

int menu,      // numéro du menu sélectionné
  mrecord,     // mémoire du reccord de vitesse pour savoir si il y à lieux
  nbt_record,  // nombre de tour record (stocké en EEprom)
  mnbt_record; // mémoire du nombre de tour record  pour savoir si il y à lieux

byte nb_menu = 4; // nombre de menu
String t_menu[] = {"Battle Nb de Tr ", "Speed Solo roue1", "Speed Solo roue2", "Montaulab ...   "};
String  texte; // texte pour les textes long qui défille
int len,pos; // longeur du texte et position du texte

int mpos_led, pos_led, dir_led=1; // varialbe pour les néopixels

int r1,r2; // calcul des vitesses de rotation des roues


/********************  setup *******************/
void setup() { 
  // EEPROM.write(0,255);
  // EEPROM.write(1,255);

  // les deux capteurs effet hall sont branchés sur 2 et 3 avec des intéruptions sur front montant
  attachInterrupt(digitalPinToInterrupt(pin_roue1), top_roue1, RISING);
  attachInterrupt(digitalPinToInterrupt(pin_roue2), top_roue2, RISING);

  pinMode( pin_roue1 , INPUT_PULLUP); // entrée du capteur effet hall roue 1
  pinMode( pin_roue2 , INPUT_PULLUP); // entrée du capteur effet hall roue 2
  
  pinMode( pin_bp1 , INPUT_PULLUP); // entrée du bouton 1
  pinMode( pin_bp2 , INPUT_PULLUP); // entrée du bouton 2
  pinMode( pin_bp3 , INPUT_PULLUP); // entrée du bouton 3

  pinMode( pin_v1 , OUTPUT); // sortie voyant du bouton 1
  pinMode( pin_v2 , OUTPUT); // sortie voyant du bouton 2
  pinMode( pin_v3 , OUTPUT); // sortie voyant du bouton 3

  lcd.init(); // initialisation de l'écran
  lcd.backlight(); // allume le rétroévlairage

  pixels.begin(); // initialisation le ruban de néopixels
  pixels.clear(); // éteint les néopixels
  pixels.show();  // active les néopixels 

  byte b0  = EEPROM.read(0);  // lecture du 1er octet dans la EEprom
  byte b1  = EEPROM.read(1);  // lecture du 2er octet dans la EEprom
  record = mrecord = b0 * 256 + b1; // extrait le record de vitesse qui était stocké sur deux octets

  nbt_record = mnbt_record = EEPROM.read(2); // extrait le record du nombre de tour

}

/********************  top_roue1 *******************/
void top_roue1(){ // intéruption lors d'un front montant du capeteur effet hall de la roue 1
  t1 =  millis() - mt1; // calcul le temps entre deux fronts montant
  mt1 = millis(); // mémorise le temp courant 
  nbt1++; // incrémente le nombre de tour 
  if(etape>1){ 
    if (t1 < max1) max1 = t1;     // mémorise le  temps le plus petit ( record personel)
    if (t1 < record) record = t1; // si un record est batu on le mémorise    
  }
}


/********************  top_roue2 *******************/
void top_roue2(){ // intéruption lors d'un front montant du capeteur effet hall de la roue 1
  t2 = millis() - mt2; // calcul le temps entre deux fronts montant
  mt2 = millis(); // mémorise le temp courant 
  nbt2++; // incrémente le nombre de tour 
  if( etape>1 and etape <5){
    if (t2 < max2) max2 = t2;     // mémorise le  temps le plus petit ( record personel)
    if (t2 < record) record = t2; // si un record est batu on le mémorise     
  }
}


/********************  loop *******************/
void loop() {
  t0 = millis(); // mémorise le temps de référence pour les tempo
  
  r1 = int( 1000.0/t1*400 ); // précalcul de la vitesse de la roue 1
  r2 = int( 1000.0/t2*400 ); // précalcul de la vitesse de la roue 2
  
  if      (etape == 0) attente();   
  else if (etape == 1) choix();
  else if (etape == 2) battle();    // jeu Battle Nb de Tr
  else if (etape == 3) speed1();    // Speed Solo roue1
  else if (etape == 4) speed2();    // Speed Solo roue1
  else if (etape == 5) montaulab(); // info montaulab
}


/********************  attente *******************/
void attente(){ // attente

  anim1(); // fait clignoter les néopixel

  clignotte(3); // faire clignotter le bouton 3

  if (bp3 ) { // selection du jeu si bouton 3
    etape = 1; // étape du jeu 
    tbp = t0 + 300; // anti rebond des boutons poussoir
    texte = "Choisir un jeu avec + ou - puis appuyez sur valider ";
    init_jeu();  // Initialise plusieurs variables 
    pixels.clear(); // éteint les néopixels 
    pixels.show(); // active les néopixels 
  } 

  lcd.setCursor(0,0); // positionne le curseur sur la première ligne du LCD
  lcd.print("Record " + String(int( 1000.0/record*400 ) )+ " mm/s   ") ;
    
  lcd.setCursor(0,1); // positionne le curseur sur la deuxième ligne du LCD
  lcd.print( "R1:" +  String( r1 ) + " R2:" +  String( r2 ) + "      "); // affiche la vitesse des roues
}


/********************  choix *******************/
void choix(){ // menu du jeu

  clignotte(1); // faire clignotter le bouton 1
  clignotte(2); // faire clignotter le bouton 2 
  clignotte(3); // faire clignotter le bouton 3

  if (bp1){ // bouton 1 menu +
    tbp = t0 + 300; // anti rebond des boutons poussoir
    menu ++;        // change le menu
    if (menu == nb_menu) menu = 0; // si fin de menu on recommance
  }
  else if (bp2){ // bouton 3 valide
    tbp = t0 + 300; // anti rebond des boutons poussoir  
    menu --;        // change le menu
    if (menu < 0) menu = nb_menu - 1; // si fin de menu on recommance
  }
  else if ( bp3 ){ // bouton 3 valide
    tbp = t0 + 300; // anti rebond des boutons poussoir
    v1_off; // éteint le voyant du bouton 1
    v2_off; // éteint le voyant du bouton 2
    v3_off; // éteint le voyant du bouton 3
    if (menu == 0) {  // battle
      etape = 2; 
      texte = "Tourner les roues pour allumer les leds  ";
      init_jeu(); // Initialise plusieurs variables 
    }
    else if (menu == 1) { // Speed solo roue1 
      etape = 3; 
      texte = "Tourner la roue 1 pour allumer la led ";
      init_jeu(); // Initialise plusieurs variables 
    }
    else if (menu == 2) {// Speed solo roue2  
      etape = 4; 
      texte = "Tourner la roue 2 pour allumer la led ";
      init_jeu(); // Initialise plusieurs variables 
    }   
    else if (menu == 3){     // montaulab
      etape = 5;
      texte = "Venez visiter notre site web ou venez nous voir sur Montauban, Notre Fablab est ouvert tout les jeudi a partir de 17h et le premier samedi du mois c'est Repair-Cafe ";
      init_jeu(); // Initialise plusieurs variables 
      pixels.clear(); // éteint les néopixels
      pixels.show();  // active les néopixels 
    } 
  }
  lcd.setCursor(0,0); // positionne le curseur sur la première ligne du LCD
  texte_long(); // fait défiler l'affichage de la variable texte
  lcd.setCursor(0,1); // positionne le curseur sur la deuxième ligne du LCD
  lcd.print(t_menu[menu] + "          "); // affiche le nom du menu en cours
}


/********************  battle *******************/
void battle(){ // Battle Nb de Tr
  if(jeu==0) { // placement des roues
    lcd.setCursor(0,0); // positionne le curseur sur la première ligne du LCD
    lcd.print("Battle R1 / R2     ");      
    lcd.setCursor(0,1); // positionne le curseur sur la deuxième ligne du LCD
    texte_long(); // fait défiler l'affichage de la variable texte
    if(roue1) v1_on; else v1_off; // éteint le voyant du bouton 1
    if(roue2) v2_on; else v2_off; // éteint le voyant du bouton 2
    if(roue1 and roue2) { // si les deux roues sont en place
      jeu=1; // démarre la tempo de 10s
      t3 = t0 + 10000; // tempo de 10s
      r1 = r2 = 0; // met à zéro les vitesses des deux roues
      anim2(); // animation des néopixels
    }     
  }
  else if (jeu==1){ // décompte avent début du jeu
    lcd.setCursor(0,0); // positionne le curseur sur la première ligne du LCD
    lcd.print("Pret a tourner  "); 
    lcd.setCursor(0,1); // positionne le curseur sur la deuxième ligne du LCD  
    lcd.print("dans : " + String(1 + (t3 - t0)/1000 ) + "                     ");   
    if (t0>t3) { // démarre le jeu
      jeu=2; // décompte avant début du jeu
      t3 = t0 + 5000;  // durée du jeu
      max1 = 99999; // init temps pour une rotation plus le temps est grand plus la vitesse est faible
      r1 = r2 = 0;  // initialise les vitesses de rotation des roues 
      nbt1 = nbt2 = 0; // initialise les nombres de tour des roues 
    }
    if(!roue1 or !roue2) { // si les roues ne sont pas en position de départ
      jeu = 0; // attente placement des roues
      v1_off; // éteint le voyant du bouton 1
      v2_off; // éteint le voyant du bouton 2
      pos = 0;
      pixels.clear(); // éteint les néopixels
      pixels.show();  // active les néopixels 
      tt = t0 + 2000; // tempo 2s avant défilement du texte du LCD
    }
  } 
  else if (jeu==2){ // décompte avant début du jeu
    lcd.setCursor(0,0); // positionne le curseur sur la première ligne du LCD
    lcd.print( "R1:" +  String(nbt1 ) + " R2:" +  String( nbt2) + "      "); // un tour de roue = 400 mm

    lcd.setCursor(0,1); // positionne le curseur sur la deuxième ligne du LCD  
    lcd.print("Tournez ... " + String(1 + (t3 - t0)/1000 ) + "        ");
    if (t0>t3){
      jeu = 3; // résultat   
      int mnbt1=nbt1, mnbt2=nbt2;
      if (mnbt1 > nbt_record) nbt_record = mnbt1;
      if (mnbt2 > nbt_record) nbt_record = mnbt2;   
    }
    anim2();
  }
  else if (jeu==3){ // fin du jeu résultat
    v1_off; // éteint le voyant du bouton 1
    v2_off; // éteint le voyant du bouton 2
    lcd.setCursor(0,0); // positionne le curseur sur la première ligne du LCD
    lcd.print("record " + String( nbt_record ) + " tours       ");
    lcd.setCursor(0,1); // positionne le curseur sur la deuxième ligne du LCD
    lcd.print("R1:" + String( nbt1) + " R2:" + String( nbt2 ) + "        "); 
    if (nbt1<nbt2)  // si roue 2 gagne
      for (int i=0; i<nb_led; i++) 
        if (i<11)pixels.setPixelColor( i , 0x00aa00 ); 
        else pixels.setPixelColor( i , 0 );
    else if (nbt1>nbt2) // si roue 1 gagne
      for (int i=0; i<nb_led; i++) 
        if (i>11)pixels.setPixelColor( i , 0x00aa00 ); 
        else pixels.setPixelColor( i , 0 );        
    else  // si égalité
      for (int i=0; i<nb_led; i++) 
        pixels.setPixelColor( i , 0x00aa00  );       
    
    pixels.show(); // active les néopixels 
    t3 = t0 + 10000; // 10 scondes
    jeu = 4; // mode attente
    if (nbt_record != mnbt_record) save_nbt_record(); // mémorise le nombre de tour record dans la EEprom
  }
  else if (jeu==4){ // attente avant de redémarrer
    if(t0>t3) {
      etape = 0;
      pos=0;
    }  
  }

} // fin battle 2


/********************  speed1 *******************/
void speed1(){ // Speed solo roue1  

  if(jeu==0) { // placement de la roue 1
    lcd.setCursor(0,0); // positionne le curseur sur la première ligne du LCD
    lcd.print("Speed solo roue1 ");      
    lcd.setCursor(0,1); // positionne le curseur sur la deuxième ligne du LCD
    texte_long(); // fait défiler l'affichage de la variable texte    
    if(roue1) {
      jeu=1; 
      t3 = t0 + 10000;
      v1_on;
    }     
  } 
  else if (jeu==1){ // décompte avent début du jeu
    lcd.setCursor(0,0);  // positionne le curseur sur la première ligne du LCD
    lcd.print("Pret a tourner  "); 
    lcd.setCursor(0,1); // positionne le curseur sur la deuxième ligne du LCD  
    lcd.print("dans : " + String(1 + (t3 - t0)/1000 ) + "                     ");   
    if (t0>t3) { // start du jeu 1
      jeu=2; 
      t3 = t0 + 5000;  
      max1 = 99999; // temps pour une rotation
    }
    if(!roue1) {
      jeu = 0;
      v1_off; // éteint le voyant du bouton 1
      pos = 0;
      tt = t0 + 2000; // tempo 2s avant défilement du texte du LCD
    }
  }
  else if (jeu==2){ // décompte avant début du jeu
    lcd.setCursor(0,0);
    lcd.print("r1 = ");
    lcd.print( r1 ); // un tour de roue = 400 mm
    lcd.print(" mm/s         ");
    lcd.setCursor(0,1); // positionne le curseur sur la deuxième ligne du LCD  
    lcd.print("Tournez ... " +  String(1 + (t3 - t0)/1000 )+ "       ");
    if (t0>t3){
      jeu = 3;
      t3 = t0 + 10000;
      if (record != mrecord ) save_record(); // mémorise la vitesse record dans la EEprom        
    }
  }
  else if (jeu==3){ // résultat
    lcd.setCursor(0,0); // positionne le curseur sur la première ligne du LCD
    lcd.print("record " + String(int( 1000.0/record*400 )) + " mm/s         ");
    lcd.setCursor(0,1); // positionne le curseur sur la deuxième ligne du LCD
    lcd.print("vous = " + String(int( 1000.0/max1*400 )) + " mm/s         "); 
    if(t0>t3) {
      v1_off; // éteint le voyant du bouton 1
      etape = 0;
    }   
  }
 
}


/********************  speed2 *******************/
void speed2(){ // Speed solo roue2 

  if(jeu==0) { // placement de la roue 2
    lcd.setCursor(0,0);
    lcd.print("Speed solo roue2 ");      
    lcd.setCursor(0,1); // positionne le curseur sur la deuxième ligne du LCD
    texte_long();  // fait défiler l'affichage de la variable texte
    if(roue2) {
      jeu=1; 
      t3 = t0 + 10000;
      v2_on;
    }     
  } 
  else if (jeu==1){ // décompte avent début du jeu
    lcd.setCursor(0,0);   // positionne le curseur sur la première ligne du LCD
    lcd.print("Pret a tourner  "); 
    lcd.setCursor(0,1); // positionne le curseur sur la deuxième ligne du LCD  
    lcd.print("dans : " + String(1 + (t3 - t0)/1000 ) + "                     ");   
    if (t0>t3) { // start du jeu 1
      jeu=2; 
      t3 = t0 + 5000;  
      max2 = 99999; // temps pour une rotation
      pos_led = 10;
    }
    if(!roue2) {
      jeu = 0;
      v2_off;
      pos = 0;
      tt = t0 + 2000; // tempo 2s avant défilement du texte du LCD
    }
  }
  else if (jeu==2){ // décompte avent début du jeu
    lcd.setCursor(0,0);
    lcd.print("r2 = ");
    lcd.print( r2 ); // un tour de roue = 400 mm
    lcd.print(" mm/s         ");
    lcd.setCursor(0,1); // positionne le curseur sur la deuxième ligne du LCD  
    lcd.print("Tournez ... " +  String(1 + (t3 - t0)/1000 )+ "       ");
    if (t0>t3){
      jeu = 3;
      t3 = t0 + 10000;
      if (record != mrecord ) save_record(); // mémorise la vitesse record dans la EEprom       
    }
  }
  else if (jeu==3){ // décompte avent début du jeu
    lcd.setCursor(0,0); // positionne le curseur sur la première ligne du LCD
    lcd.print("record " + String(int( 1000.0/record*400 )) + " mm/s         ");
    lcd.setCursor(0,1); // positionne le curseur sur la deuxième ligne du LCD
    lcd.print("vous = " + String(int( 1000.0/max2*400 )) + " mm/s         "); 
    if(t0>t3) {
      etape = 0;
    }   
  }
  
}

/******************** montaulab  *******************/
void montaulab(){ // fait clignotter les boutons
  clignotte(3);
 

  uint32_t coul[]={0x330000 , 0x000033 , 0x003300 , 0x222200 , 0x220022 , 0x002222,0x222222}; // mémo de la couleur
  if( bp3 ) {
    etape = 0;
    tbp = t0 + 300; // anti rebond des boutons poussoir
  }
  if (t0>t3) {
    t3 = t0 + 100;
    pos_led = random(nb_led);
    pixels.setPixelColor( mpos_led , 0  );  
    pixels.setPixelColor( pos_led , coul[ random(7) ] );       
    mpos_led = pos_led;
    pixels.show(); // active les néopixels 
  }
  if(jeu == 0){
    jeu = 1;
    lcd.setCursor(0,0); // positionne le curseur sur la première ligne du LCD
    lcd.print(" montaulab.com  "); 
  }
 lcd.setCursor(0,1); // positionne le curseur sur la deuxième ligne du LCD
 texte_long(); // fait défiler l'affichage de la variable texte

}


/******************** clignotte  *******************/
void clignotte(int v){ // fait clignotter les voyants des boutons
  if(v==1){
    if(t0>tv1){
      tv1 = t0 + 100;
      if(v1) v1_off; else v1_on; // inverse l'état du voyant du bouton 1
    }
  } else if(v==2){
    if(t0>tv2){
      tv2 = t0 + 100;
      if(v2) v2_off; else v2_on; // inverse l'état du voyant du bouton 2
    }
  } else if(v==3){
    if(t0>tv3){
      tv3 = t0 + 100;
      if(v3) v3_off; else v3_on; // inverse l'état du voyant du bouton 3
    }
  }
}


/********************  texte_long *******************/
void texte_long(){  // fait défiler l'affichage de la variable texte
  lcd.print( texte.substring(pos, 16+pos) ); 
  if (t0>tt){
    tt=t0+300;
    pos++;
    if (pos > len+16){
      pos = 0;
      tt=t0+2000;
    }
  }
}


/******************** init_jeu  *******************/
void init_jeu(){ // reset des variables
  len = texte.length()-16;
  pos=0;
  tt = t0 + 2000; // tempo 2s avant défilement du texte du LCD
  nbt1 = nbt2 = jeu = 0;
  v1_off;
  v2_off; 
  v3_off; 
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
    pixels.show(); // active les néopixels 
  }
}


/******************** anim2  *******************/
void anim2(){ // animation des néopixels

  pos_led = 10 + (r1 - r2)/100;

  if(pos_led>nb_led)pos_led = nb_led-2;
  else if(pos_led<1)pos_led = 0;
  pixels.setPixelColor( mpos_led ,0  );
  pixels.setPixelColor( mpos_led + 1 ,0  );  

  pixels.setPixelColor( pos_led , 0x00ff00  );
  pixels.setPixelColor( pos_led +1 , 0x00ff00  );

  mpos_led = pos_led;

  pixels.show(); // active les néopixels 

}

void save_record(){
  byte b0 =  record / 256;              // 1er octet de la puissance
  byte b1 =  record - (b0 * 256);         // 2eme octet de la puissance
  EEPROM.write( 0, b0 );
  EEPROM.write( 1, b1 );
  mrecord = record;
}

void save_nbt_record(){
  EEPROM.write( 2, nbt_record );
  mnbt_record = nbt_record;
}

