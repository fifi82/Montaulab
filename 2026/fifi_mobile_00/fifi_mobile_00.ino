
#include <WiFi.h>
#include <WebServer.h>

// SSID & Password
const char* ssid = "fifi-esp32";  // Enter your SSID here
const char* password = "";  //Enter your Password here

// IP Address details
IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

WebServer server(80);  // Object of WebServer(HTTP port, 80 par défaut)

String p; // contient la page web à afficher

int v_mini,v_maxi; // vitesse ini et maxi de la voiture


void setup() {
  Serial.begin(115200);

  pinMode(4,OUTPUT); // pour exemple

  // Create SoftAP
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);

  Serial.print("connection a mon point d'acces: ");
  Serial.println(ssid);


  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
 
    int paramsNr = request->params();
    Serial.println(paramsNr);
 
    for(int i=0;i<paramsNr;i++){
 
        AsyncWebParameter* p = request->getParam(i);
        Serial.print("Param name: ");
        Serial.println(p->name());
        Serial.print("Param value: ");
        Serial.println(p->value());
        Serial.println("------");
    }
 
    request->send(200, "text/plain", "message received");
  });
  
  server.on("/", page_start);
  server.on("/led", led);
  server.on("/v_mini", led );

  server.begin();
  Serial.println("HTTP server started");
  delay(100);

 
}

void loop() {
  server.handleClient();
  

}

// Handle root url (/)
void page_start() {
  page_0();
  server.send(200, "text/html", p);
  Serial.println("page start");
}


void led(){
  if (digitalRead(4)) {
    digitalWrite(4,0); 
    Serial.println("led off");
  } else {
    digitalWrite(4,1);
    Serial.println("led on");
  }

  page_0();
  server.send(200, "text/html", p);
  
}

void _v_mini(){


}

/////////////////////////////////////////////// gestion des pages html //////////////////////////////////////////

/*##################################################  page_0 ###################################################*/
String page_0(){ // page HTML

  String t; // permetra de stocker du texte

  entete("Montaulab TRR 2026<br>" , 5 ); // cré une nouvelle page qui se rafraichi toute les 5s 

  titre( "données de la meilleure voiture", "h2" , "#33aa55", "#FFFFFF");

  br();
  if(digitalRead(4)) t = "éteindre la led";  else t = "allumer la led"; // modifi le texte en fonctionde l'état de la led
  bouton( t ,"led","aaaaaa","24"); br();   // affiche le bouton de la led

  
  
  br(); // retour à la ligne
  
  fin_page();  // fin de page HTML
  return p;
} // fin page_0()


/*################################################## page_v_mini ###################################################*/
/*
String page_v_mini(){ // vitesse mini
  entete("Montaulab TRR 2026<br>" , 0 );
  titre("vitesse mini de la voiture 90 à 180");
  p += "de 90 à 180, valeur en cours : " + String(v_mini) + "<br>";
  p += "<form> <input type=\"number\" name=\"v_mini\" id=\"coucou\" min=\"90\" max=\"180\"  /><br>";
  p += "<input type=\"submit\" value=\"valider\"> </form> <br>";

  bouton("Retour","","aaaaaa","24"); 
  

  fin_page(); 
  return p;
}
*/



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

