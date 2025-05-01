# le jeu de la roue folle


le but est de tourner la roue le plus vite possible ...

## vue d'enssemble:
<img alt="roue folle" src="https://github.com/fifi82/Montaulab/blob/main/le_jeu_de_la_Roue_folle/photo1.JPG" /><br><br>
## le cablage:
1 régulateur 7805 de 5v<br>
1 LCD i2c 16x2<br>
1 Arduino uno (ou nano)<br>
22 WS2812b leds néopixel<br>
3 boutons poussoir avec led<br>
2 capteurs effet hall 3144<br>
2 roues diam 127mm ( environ 400mm de périmètre )<br>
<img alt="roue folle" src="https://github.com/fifi82/Montaulab/blob/main/le_jeu_de_la_Roue_folle/schema_bb.png" /><br><br>

### Notice
- Attente : Appuyer sur « Valider » pour démarrer un jeu.<br> 
	À la mise sous tension le bouton « Valider » clignote ainsi que le ruban de leds,	Le texte affiche le record de tout les temps durant un jeu,	la vitesse est déterminée par la dimension de la roue qui fait 125 mm de diamètre, soit 400 mm de déplacement pour 1 tour environ.

- Sélection : Appuyer sur « + » ou « - » pour choisir un jeu, puis sur « valider »<br>
-Battle Nb de Tr<br>
-Speed Solo roue 1<br>
-Speed Solo roue 2<br>
-Montaulab …<br>

- Battle Nb de Tr : tournez les roues pour allumer les voyants des boutons poussoir, attendre le 	décompte de 10 secondes, puis tournez les roues pour faire le plus de tours possible pendant	5 secondes. Que le meilleur gagne.

- Speed Solo roue 1 : tournez la roue 1 pour allumer le  voyant du bouton  poussoir « + », attendre le décompte de 10 secondes, puis tournez la roue le plus vite possible pour établir un nouveau record.

- Speed Solo roue 2 : tournez la roue 2 pour allumer le  voyant du bouton  poussoir « - », attendre le 	décompte de 10 secondes, puis tournez la roue le plus vite possible pour établir un nouveau 	record.

- Montaulab … : Affichage du site web du fablab, https://montaulab.com/<br><br>
