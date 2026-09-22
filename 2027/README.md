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
Il est possible de tester le programme avec les flèches du clavier du PC sans la com avec la voiture.<br>

<br>vue de la version 3.4
<img alt="piste 3D" src="https://github.com/fifi82/Montaulab/blob/main/2027/visu_3.4_.jpg" /><br>
<br>
<br>
les fichier .7z  contiennent les dépendances (son, objets 3D et textures)<br>

le fichier "piste_3D_python_v3.5_.7z" à une vue de dessus sur la droite<br>
le fichier "piste_3D_python_v3.4_.7z" à un menu sur la droite<br>
<br>
<br>
<hr>
