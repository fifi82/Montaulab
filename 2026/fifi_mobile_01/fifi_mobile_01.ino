
#include <WiFi.h>       // pour la gestion du wifi
#include <EEPROM.h>     // EEprom pour la sauvegarde des datas

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

/************************************************* setup *************************************************/
void setup() {
  Serial.begin(115200); // port série pour le débug

  EEPROM.begin(10); // réserve 10 octets dans la EEprom
  
  v_mini = ee_read(0); // lit la valeur v mini de la eeprom à l'adresse 0
  v_maxi = ee_read(2); // lit la valeur v maxi de la eeprom à l'adresse 2

  pinMode(4,OUTPUT); // pour exemple
  led();

  // Create SoftAP
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);

  Serial.print("connection a mon point d'acces: ");
  Serial.println(ssid);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP adresse IP : ");
  Serial.println(IP);
  
  server.begin();
  Serial.println("demarrage du serveur");
  delay(100);

}

/************************************************* loop *************************************************/
void loop() {
  client = server.available();  // regarde si il y à une connections
  if ( client ) wifi();         // si un nouveau client,
  
  


}

/************************************************* wifi *************************************************/
void wifi(){
  delay(10);

    String request = client.readStringUntil('\r'); // récupère la requette
    client.flush(); // efface le tampon pour les prochaines requettes

    if (request.indexOf("/led") != -1 ) { // si la requette est /led
      led(); // inverse la led
      p_0(); // réaffiche la page0
    }
    else if (request.indexOf("?v_mini") != -1 ) { // si la requette est /led
      v_mini = val(request);
      ee_write(v_mini , 0); // enregistre v_mini dans la EEprom à l'adresse 0
      p_0(); // affiche la page web 0
    }
    else if (request.indexOf("/b_v_mini") != -1 ) { // si la requette est /led
      client.println( page_v_mini() ); // réaffiche la page0
    }
    else if (request.indexOf("?v_maxi") != -1 ) { // si la requette est /led
      v_maxi = val(request);
      ee_write(v_maxi , 2); // enregistre v_mini dans la EEprom à l'adresse 0
      p_0(); // affiche la page web 0
    }
    else if (request.indexOf("/b_v_maxi") != -1 ) { // si la requette est /led
      client.println( page_v_maxi() ); // réaffiche la page0
    }
    else p_0(); // affiche la page web 0
    

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


/*################################################## page_v_mini ###################################################*/
String page_v_mini(){ // vitesse mini

  entete("Montaulab TRR 2026<br>" , 0 );
  titre("vitesse mini 90 à 180" , "h2" , "#33aa55", "#FFFFFF" );
  p += "de 90 à 180, valeur en cours : " + String(v_mini) + "<br>";
  p += "<form> <input type=\"number\" name=\"v_mini\" id=\"coucou\" min=\"90\" max=\"180\" size=\"20\" /><br>";
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
  p += "<form> <input type=\"number\" name=\"v_maxi\" id=\"coucou\" min=\"90\" max=\"180\" size=\"20\" /><br>";
  p += "<input type=\"submit\" value=\"  valider  \"> </form> <br>";

  bouton(" Retour ","","dddddd","24"); 
  
  fin_page(); 
  return p;
}

// fonction pour gérer le HTML plus facilement
/*##################################################  entête  ###################################################*/
void entete(String texte, int r){// header texte=titre de la fenetre, r = au temps en seconde avant de rafraichir la fenetre
   // entête des pages html 
  p = "<!doctype html>\n";
  p += "<head>\n";
  p += "<meta charset=\"utf-8\">\n";
  p += "<title>Solaire - fifi82 2025</title>\n";
  if(r) p += "<meta http-equiv=\"refresh\" content=\"" + String(r) + ";/\">\n"; // permet de recharger la page toute les r secondes
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

