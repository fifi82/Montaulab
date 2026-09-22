# Montaulab

<hr>

### Projet TRR 2027
<br>
Programme Python et Panda3D qui représente la piste du TRR de Toulouse.<br>
Cela permet d'entrainer les voitures autonome qui se repère avec une caméra sur un circuit virtuel.<br>
La communication entre la voiture et le PC se fait via le port USB/Série avec 3 octets :<br>
- octet n°1 = direction, valeur de 0 à 180 (format d'un servomoteur)<br>
- octet n°2 = vitesse, valeur de 0 à 180 (format d'un servomoteur) <br>
- octet n°3 = Synchro, valeur255 <br>
  <br>
Il est possible de tester le programme avec les flèches du clavier du PC
<br>
<img alt="piste 3D" src="https://github.com/fifi82/Montaulab/blob/main/2027/visu_3.4_.jpg" /><br>

<hr>

<br>

<hr>

