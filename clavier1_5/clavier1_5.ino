/* fifi82 2025 

 le clavier sera branché à partir de la broche 2 dans cet exemple, 4 entrées et 4 sorties
   bornes 2,3,4,5 sont des entrées et 6,7,8,9 des sorties
 */ 

const int bclav = 2;   // le clavier sera branché à partir de la broche 2, de 2 à 9

int touche,mtouche; // état du clavier et sa mémoire

int type = 1; // type de clavier utilisé

// valeurs à afficher en fontion du type de clavier
// si il y à un décallage c'est les pins qui ne sont pas branchées pareil, il suffit d'adapter les valeurs ci-dessous
String clav1[] = {"1","4","7","*",
                  "2","5","8","0",
                  "3","6","9","#",
                  "A","B","C","D"};

String clav2[] = {"E","F","G","*",
                  "I","J","K","L",
                  "M","N","O","P",
                  "g","coucou","bonjour","fin"};    // un touche peut comptenir aussi du texte               

/***************************** setup **********************/
void setup() {

  Serial.begin(9600);

   // positionne les pins en entrées ou en sorties
  for (byte i=bclav;   i<bclav+8; i++) // les 8 pins ou seront branchées le clavier
    if (i<bclav+4) pinMode(i, INPUT_PULLUP); // 4 premières broches en entrées 
    else pinMode(i, OUTPUT);                 // 4 suivantes en sorties
  
  Serial.println( "Appuyez sur [*] pour changer le clavier");
}

/********************** loop **************************/
void loop(){
  
  f_clav(); // lecture du clavier, de 0 à 15
    
  if (touche != mtouche){ // si évenement du clavier
    mtouche = touche; // mémorise la touche appuyé pour éviter les répétitions
    if (touche > -1 ){ // si une touche est appuyée

      Serial.print( "N° de touche = " );
      Serial.print( touche ); // affiche le numéro de la touche
      Serial.print( ", String assosié = " );
     
      if ( type == 1 ) { // type du clavier 1
        Serial.println( clav1[touche] ); // affiche le string de clav1
        if ( touche == 3 ){// change le clavier avec *
          type = 2;
          Serial.println("Clavier type 2");
        } 
      } else if ( type == 2 ) { // type du clavier 2
        Serial.println( clav2[touche] );// affiche le string de clav2
        if ( touche == 3 ) { // change le clavier avec *
          type = 1;
          Serial.println("Clavier type 1");  
        }        
      } 

    }// if (touche > -1 )
  } // if (touche != mtouche)
 
} // loop 


/************************ fclav ******************************/
int f_clav() {  // fonction f_clav 
  touche = -1; // si pas de touche appuyée on retourne -1
  for (byte x=0;  x<4; x++){  // passe en revu les 4 entrées - colonnes
    for (byte y=0; y<4; y++) { // passe en revu les 4 sorties - lignes
      int my = bclav + y + 4;
      digitalWrite(my, 0); // positionne à 0 chaques ligne 
      if (!digitalRead(bclav + x)) touche = x+4*y; // état du clavier de 0 à 15
      digitalWrite(my, 1);  // re-positionne à 1 la sortie
    }
  } 
}   
