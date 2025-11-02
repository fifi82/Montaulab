
#include <WiFi.h>       // pour la gestion du wifi
#include <EEPROM.h>     // EEprom pour la sauvegarde des datas
#include <VL53L1X.h>
#include <ESP32Servo.h>

// les servomoteurs
Servo servo_direction;
Servo servo_vitesse;

VL53L1X lidar_centre;
VL53L1X lidar_droit;
VL53L1X lidar_gauche;

int minUs = 1000;
int maxUs = 2000;
int pin_servo_direction = 16;
int pin_servo_vitesse = 15;

// SSID & Password
const char* ssid = "le_Wi_Fifi";  // Enter your SSID here
const char* password = "123456789";  //Enter your Password here

// IP Address details
IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

WiFiServer server(80);  // objet pour le serveur web
WiFiClient client;      // prépare la récetion du client

String p; // contient la page web à afficher

int v_mini,v_maxi; // vitesse ini et maxi de la voiture
int d_centre;   // centre du servo de direction par défaut 90
int d_droite; // maxi doite du servo de direction par défaut 180
int d_gauche; // maxi gauche du servo de direction par défaut 0
int nb_tour_a_faire = 5;
int nb_tour = 0; // nombre de tour à faire
long nb_roue = 0;
byte hall = false;

int refresh = 0;

int direction = 90;
int vitesse = 90;
int m_direction = 90;
int m_vitesse = 90;

int lidar; // différence entre lidar droit et gauche en cm

/************************************************* setup *************************************************/
void setup() {
  Serial.begin(115200); // port série pour le débug

  EEPROM.begin(20); // réserve 10 octets dans la EEprom
  
  v_mini = ee_read(0); // lit la valeur v_mini de la eeprom à l'adresse 0
  v_maxi = ee_read(2); // lit la valeur v_maxi de la eeprom à l'adresse 2
  d_centre = ee_read(4); // lit la valeur d_centre de la eeprom à l'adresse 4
  d_droite = ee_read(6); // lit la valeur d_droite de la eeprom à l'adresse 6 
  d_gauche = ee_read(8); // lit la valeur d_gauche de la eeprom à l'adresse 8
  nb_tour_a_faire = ee_read(10); // lit la valeur d_gauche de la eeprom à l'adresse 10

  // pour ESP neuve:
  if (v_mini>255) v_mini=0;
  if (v_maxi>255) v_maxi=150;
  if (d_centre>255) d_centre=90;
  if (d_droite>255) d_droite=180;
  if (d_gauche>255) d_gauche=0;

  pinMode(4,OUTPUT); // pour exemple
  led(); // inverse l'état de la led

  WiFi.softAP(ssid, password);     // cré un point acces avec les codes défini avant
  WiFi.softAPConfig(local_ip, gateway, subnet); // configure les adresse ip du serveur

  Serial.print("connection a mon point d'acces: ");
  Serial.println(ssid);

  IPAddress IP = WiFi.softAPIP(); // récupère l'adresse ip du serveur
  Serial.print("AP adresse IP : ");
  Serial.println(IP);
  
  server.begin();
  Serial.println("demarrage du serveur");
  delay(100);

  ESP32PWM::allocateTimer(0);
	ESP32PWM::allocateTimer(1);
	ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  servo_direction.attach(pin_servo_direction, minUs, maxUs);
	servo_vitesse.attach(pin_servo_vitesse, minUs, maxUs);  

  servo_vitesse.write(vitesse);
  servo_direction.write(direction);

  pinMode(13,INPUT_PULLUP); // capteur effet Hall de la roue arrière droite

  init_lidar();
}

/************************************************* loop *************************************************/
void loop() {
  client = server.available();  // regarde si il y à une connections
  if ( client ) wifi();         // si un nouveau client,
  


  if (vitesse != m_vitesse) {
  //  servo_vitesse.write(vitesse);
    m_vitesse = vitesse;
  }

  direction = (lidar_droit.read()- lidar_gauche.read())/5 + 90;

  if (direction >d_droite) direction = d_droite;
  else if(direction < d_gauche) direction = d_gauche;

  

  if (direction != m_direction){
    //delay(1);
   // Serial.println(direction);
    servo_direction.write(direction);
    m_direction = direction;
  }
  
  
  if(digitalRead(13)){
    if(!hall) {
      nb_roue++;
      hall = true;    
    }
  } else hall = false;

}

//////////////////////////////// init_lidar ///////////////////////////////
void init_lidar(){
    pinMode(18,OUTPUT);
    digitalWrite(18,0);
    pinMode(19,OUTPUT);
    digitalWrite(19,0);
    delay(100);
    
    Wire.begin(21, 22);
    delay(100);

    lidar_centre.setTimeout(500);
    if (!lidar_centre.init()) {
        Serial.println("defaut lidar_centre");
        while (1);
    }
    lidar_centre.setAddress(0x2B);
    lidar_centre.startContinuous(50);

    delay(100);
    digitalWrite(18,1);
    delay(100);

    lidar_droit.setTimeout(500);
    if (!lidar_droit.init()) {
        Serial.println("Defaut lidar_droit");
        while (1);
    }
    lidar_droit.setAddress(0x2C);
    lidar_droit.startContinuous(50);
    
    delay(100);
    digitalWrite(19,1);
    delay(100);

    lidar_gauche.setTimeout(500);
    if (!lidar_gauche.init()) {
        Serial.println("Defaut lidar_gauche");
        while (1);
    }
    lidar_gauche.startContinuous(50);

}

/************************************************* wifi *************************************************/
void wifi(){
  //delay(10);
    refresh = 1;
    String request = client.readStringUntil('\r'); // récupère la requette
    client.flush(); // efface le tampon pour les prochaines requettes
   
    if (request.indexOf("/led") != -1 ) { // si la requette est /led
      led(); // inverse la led
      p_0(); // réaffiche la page0
    }
    else if (request.indexOf("?v_dir") != -1 ) { // si la requette est v_mini
      direction = val(request);
      //ee_write(v_mini , 0); // enregistre v_mini dans la EEprom à l'adresse 0
      p_0(); // affiche la page web 0
    }
    else if (request.indexOf("?v_vit") != -1 ) { // si la requette est /led
      vitesse = val(request);
      //ee_write(v_maxi , 2); // enregistre v_maxi dans la EEprom à l'adresse 2
      p_0(); // affiche la page web 0
    }


    else if (request.indexOf("?v_mini") != -1 ) { // si la requette est v_mini
      v_mini = val(request);
      ee_write(v_mini , 0); // enregistre v_mini dans la EEprom à l'adresse 0
      p_0(); // affiche la page web 0
    }
    else if (request.indexOf("?v_maxi") != -1 ) { // si la requette est /led
      v_maxi = val(request);
      ee_write(v_maxi , 2); // enregistre v_maxi dans la EEprom à l'adresse 2
      p_0(); // affiche la page web 0
    }
    else if (request.indexOf("?d_centre") != -1 ) { // si la requette est /led
      d_centre = val(request);
      ee_write(d_centre , 4); // enregistre d_centre dans la EEprom à l'adresse 4
      p_0(); // affiche la page web 0
    }    
    else if (request.indexOf("?d_droite") != -1 ) { // si la requette est /led
      d_droite = val(request);
      ee_write(d_droite , 6); // enregistre d_droite dans la EEprom à l'adresse 6
      p_0(); // affiche la page web 0
    }     
    else if (request.indexOf("?d_gauche") != -1 ) { // si la requette est /led
      d_gauche = val(request);
      ee_write(d_gauche , 8); // enregistre d_gauche dans la EEprom à l'adresse 8
      p_0(); // affiche la page web 0
    }         
    else if (request.indexOf("?ntour") != -1 ) { // si la requette nb_tour_a_faire
      nb_tour = val(request);
      //ee_write(nb_tour , 10); // enregistre nb_tour dans la EEprom à l'adresse 10
      p_0(); // affiche la page web 0
    }      
    else if (request.indexOf("?nb_tour") != -1 ) { // si la requette est nombre de tour de circuit fait
      nb_tour = val(request);
      //ee_write(nb_tour , 10); // enregistre nb_tour dans la EEprom à l'adresse 10
      p_0(); // affiche la page web 0
    }    
    else if (request.indexOf("?nb_roue") != -1 ) { // si la requette est nombre de tour de roue
      nb_roue = val(request);
      //ee_write(nb_tour , 10); // enregistre nb_tour dans la EEprom à l'adresse 10
      p_0(); // affiche la page web 0
    }  
    else {
      refresh = 0;
      p_0();
     }// affiche la page web 0
    

  client.println();
  client.stop();
}

int val(String requette){
  int a = requette.indexOf("=") ;
  requette.remove(0, a + 1);
  a = requette.indexOf(" ") ;
  requette.remove(a);
  return requette.toInt(); // récupère la valeur numérique
}

/************************************************* ee_write *************************************************/
void ee_write(int data, int adresse){ // écrit la EEprom
  EEPROM.write( adresse , data / 256 );  // écrit la valeur dans la EEprom octet 0
  EEPROM.write( adresse + 1 , data - ( data * 256 ) );  // écrit la valeur dans la EEprom octet 1   
  EEPROM.commit();
}  

/************************************************* ee_read *************************************************/
int ee_read(int adresse){ // lit la EEprom et retourn le résultat
 return 256 * EEPROM.read( adresse ) + EEPROM.read( adresse + 1 );
}

/************************************************* p_0 *************************************************/
void p_0(){
  delay(1);
  client.println( page_0() );
}

/************************************************* led *************************************************/
void led(){
 
  if (digitalRead(4)) {
    digitalWrite(4,0); 
    Serial.println("led off");
  } else {
    digitalWrite(4,1);
    Serial.println("led on");
  }
}


/////////////////////////////////////////////// gestion des pages html //////////////////////////////////////////


/*##################################################  page_0 ###################################################*/
String page_0(){ // page HTML

  String t; // permetra de stocker du texte

  entete("Montaulab TRR 2026<br>" , refresh ); // cré une nouvelle page qui se rafraichi toute les 5s 

  titre( "données de la meilleure voiture", "h2" , "#33aa55", "#FFFFFF");

  br();
  if(digitalRead(4)) t = "éteindre la led";  else t = "allumer la led"; // modifi le texte en fonctionde l'état de la led
  bouton( t ,"led","dddddd","18");  // affiche le bouton de la led
  br(); br();

  input("direction ( de 0 à 180 ) = " , direction, "v_dir"); // input(texte, valeur à modifier, nom de la requette)

  br(); br(); 

  input("vitesse ( de 0 à 180 ) = " , vitesse, "v_vit"); // input(texte, valeur à modifier, nom de la requette)

  br(); br(); 

  input("vitesse mini ( de 0 à 180 ) = " , v_mini, "v_mini"); // input(texte, valeur à modifier, nom de la requette)

  br(); br(); 

  input("vitesse maxi ( de 0 à 180 ) = " , v_maxi, "v_maxi"); // input(texte, valeur à modifier, nom de la requette)

  br(); br(); 

  input("angle de direction au centre (défaut 90) = " , d_centre, "d_centre"); // input(texte, valeur à modifier, nom de la requette)

  br(); br();

  input("angle de direction maxi à droite (défaut 180) = " , d_droite, "d_droite"); // input(texte, valeur à modifier, nom de la requette)

  br(); br();  

  input("angle de direction maxi à gauche (défaut 0) = " ,d_gauche, "d_gauche"); // input(texte, valeur à modifier, nom de la requette)

  br(); br();  

  input("nombre de tour de circuit à faire = " , nb_tour_a_faire, "ntour"); // input(texte, valeur à modifier, nom de la requette)

  br(); br();  

  input("nombre de tour de circuit fait = " , nb_tour, "nb_tour"); // input(texte, valeur à modifier, nom de la requette)

  br(); br();   

  input("nombre de tour de roue = " , nb_roue, "nb_roue"); // input(texte, valeur à modifier, nom de la requette)

  br(); br(); 
  bouton(" refresh " , "","dddddd","18" );
 
  fin_page();  // fin de page HTML
  return p;
} // fin page_0()


/*##################################################  page_0 ###################################################*/
/*
String page_0(){ // page HTML

  String t; // permetra de stocker du texte

  entete("Montaulab TRR 2026<br>" , 0 ); // cré une nouvelle page qui se rafraichi toute les 5s 

  titre( "données de la meilleure voiture", "h2" , "#33aa55", "#FFFFFF");

  br();
  if(digitalRead(4)) t = "éteindre la led";  else t = "allumer la led"; // modifi le texte en fonctionde l'état de la led
  bouton( t ,"led","dddddd","18");  // affiche le bouton de la led
  br(); br();
  bouton("vitesse minimale : " + String(v_mini) , "b_v_mini","dddddd","24" );
  br(); 
  bouton("vitesse maximale : " + String(v_maxi) , "b_v_maxi","dddddd","24" );
  br(); br(); br(); 

  bouton(" refresh " , "","dddddd","18" );
 
  fin_page();  // fin de page HTML
  return p;
} // fin page_0()
/*

/*################################################## page_v_mini ###################################################*/
String page_v_mini(){ // vitesse mini

  entete("Montaulab TRR 2026<br>" , 0 );
  titre("vitesse mini 90 à 180" , "h2" , "#33aa55", "#FFFFFF" );
  //p += "de 90 à 180, valeur en cours : " + String(v_mini) + "  : ";
  p += "<form>vitesse mini  <input type=\"number\" name=\"v_mini\" id=\"coucou\"  value=\"" + String(v_mini) + "\" /><br>";
  p += "<input type=\"submit\" value=\"  valider  \"> </form> <br>";

  bouton(" Retour ","","dddddd","24"); 
  
  fin_page(); 
  return p;
}

/*################################################## page_v_maxi ###################################################*/
String page_v_maxi(){ // vitesse mini
  entete("Montaulab TRR 2026<br>" , 0 );
  titre("vitesse maxi 90 à 180" , "h2" , "#33aa55", "#FFFFFF" );
  p += "de 90 à 180, valeur en cours : " + String(v_maxi) + "<br>";
  p += "<form> <input type=\"number\" name=\"v_maxi\" id=\"coucou\" minlength=\"90\" maxlength=\"180\" size=\"20\" /><br>";
  p += "<input type=\"submit\" value=\"  valider  \"> </form> <br>";

  bouton(" Retour ","","dddddd","24"); 
  
  fin_page(); 
  return p;
}

// fonction pour gérer le HTML plus facilement
/*##################################################  entête  ###################################################*/
void entete(String texte, int temps){// header texte=titre de la fenetre, r = au temps en seconde avant de rafraichir la fenetre
   // entête des pages html 
  p = "<!doctype html>\n";
  p += "<head>\n";
  p += "<meta charset=\"utf-8\">\n";
  p += "<title>Solaire - fifi82 2025</title>\n";
  if(temps) p += "<meta http-equiv=\"refresh\" content=\"" + String(temps) + ";/\">\n"; // permet de recharger la page toute les r secondes
  p += "<link rel=\"stylesheet\" href=\"style.css\">\n";
  p += "<script src=\"script.js\"></script>\n";
  p += "</head>\n";
  p += "<body><center>";
  p += "<h1><p style=\"background-color: #000000; color: white;\">" + texte + "<br>";
  p += "</h1>";
}

/*##################################################  bouton  ###################################################*/
void bouton(String texte, String requette, String couleur, String fonte){// \" affiche un " en HTML
  p +=  "<input type=\"button\" onclick=\"window.location.href = '/" + requette + "';\" value=\"\n  " + texte;
  p +=  "  \n \"style=\"background-color: #" + couleur + "; color: #F000000; font-size:"+fonte+"px\" /><br>"; 
}

/*##################################################  fin page  ###################################################*/
void fin_page(){
  p+= "<p style=\"background-color: #ffffff;color: white;\">. . . . . . . . . . . . . . . . . . . . . .</p></h2></center></body> </html>"; 
}

/*##################################################   texte ###################################################*/
void texte(String texte, String h){
  p +="<" + h + ">" + texte + "</" + h + ">" +"<br>";
}

/*##################################################   titre ###################################################*/
void titre2(String texte, String cf, String ct){ // h=taille du texte, cf=couleur du fond, ct=couleur du texte
  p += "<p style=\"background-color:"+ cf +"; color: "+ ct +";\">"+texte+"</p>";
}

/*##################################################   titre ###################################################*/
void titre(String texte, String h, String cf, String ct){ // h=taille du texte, cf=couleur du fond, ct=couleur du texte
  p += "<" + h + "><p style=\"background-color:"+ cf +"; color: "+ ct +";\">"+texte+"</p></" + h + ">";
}

/*##################################################   br ###################################################*/
void br(){ // retour à la ligne
  p += "<br>";
}

/*##################################################   input ###################################################*/
void input(String texte, int val, String name){ // 
  p += "<form>" + texte +"<input type=\"number\" name=\"" + name + "\" value=\"" + String(val) + "\" /> </form>";
}
