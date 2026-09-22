
from math import pi, sin, cos, radians # quelques fonctions mathématique
# from random import random
import serial                   # permet de communiquer avec le port serie
import threading                # pour la multi taches
from tkinter.messagebox import * # pour les messages d'erreur 
import time # gestion du temps pour les timer
from direct.showbase.ShowBase import ShowBase
from direct.task import Task
from direct.gui.DirectGui import * # pour l'interface
from panda3d.core import *
import imageio.v3 as iio # gestion du masque pour la collision
import numpy as np
import sys # pour forcer l'arret du programme
     
angle = 90    # l'angle ou va la voiture vers le portique
r_camera = -5 # rotation camera haut/bas
focale = 70   # focale de la caméra au démarage

px = 3.5669     # position de départ devant la ligne bleu
py = -1.8663    # position de départ devant la ligne bleu
pz = .15        # 15cm hauteur de la caméra

vd = 0  # vitesse de déplacement
mvd = 0 # mémoire de la vitesse de deplacement

servo_dir = 90 # angle du servo de direction
mservo_dir = 90 # mémoire de l'angle du servo de direction
s_servo_dir = 2 # senssibilité du servo plus ou moins réactif à la rotation

bascule = False # pour sychroniser la lecture usb

start = True # permet de stoper le thread du port série

cpt = 0 # compteur pour les points de la trace
lg  = 500 # longeur du tableau et de points de la ligne Trace
#tb =[] # tableau de sauvegarde pas utilisé 

# masque pour les collisions 
img = iio.imread("maillage_3D/textures/masque.png")
my,mx = img.shape 
collision = np.zeros((my, mx), dtype=np.bool) # c'éation d'un tableau booleen vide
for x in range(0,mx): # transforme l'image masque en tableau booleen
    for y in range(0,my):
        collision[my-y-1,x] = img[y,x] > 127      

cam_fixe = False # active la vue de haut

brouillard_Density = 0.2 # opacité du brouillard

brouillard_on = True # active ou pas le brouillard
bruit_on = False # affiche ou pas la tache
sky_on = True # affiche ou pas le SkyBox, l'environement
save_on = False # active la sauvegarde du trajet
trace_on = True # active l'affichage de la trace sauvegardée

t_rec = 200 # enregistre la position toutes les n millis seconde

t0,t1 = 0,0 # tempos

# communication serie
try:
    usb=0 # permet de savoir si l'arduino est branchée en usb
    ser = serial.Serial() # cré un objet port série gestion des boutons
    ser.baudrate = 115200 #9600
    ser.port ='/dev/ttyUSB0'
    ser.open() # ouvre le port série    
    usb=1 # port ok
except (OSError, serial.SerialException): # si pas d'usb on continuer sans stoper le programme
    showwarning("pas d'USB détecté", "utilisez le clavier pour se déplacer")
   
    
#-------------------------------------- serie() --------------------------------------
def serie():   	#lecture série
    global ser,servo_dir, mservo_dir, vd, mvd, p0,p1, bascule,start,tx1
    while start and usb: # si l'usb à bien été détecté
        if ser.in_waiting > 0: # attend une donnée sur le port série
            a = ord ( ser.read() ) # lit un octet
            if a == 255: # 255 = synchro
                bascule = False # bascule entre vitesse et direction
            else: # si pas data de synchro
                if bascule: # lecture vitesse
                    if(a>92 or a<88): # fourchette pour le 90° potard au centre
                        vd = (a - 90) # si pas au centre 
                    else:
                        vd = 0 # si au centre
                    
                else:
                    if(a>92 or a<88): # fourchette pour le 90° potard au centre
                        servo_dir = a # si pas au centre 
                    else:
                        servo_dir = 90 # si au centre
                    
                bascule = True
                if mservo_dir != servo_dir: # évite de réafficher si la valeur ne change pas
                    mservo_dir = servo_dir
                    tx1.setText('Direction = ' + "%.3f" % servo_dir) # affiche l'angle du servo
                if mvd != vd: # évite de réafficher si la valeur ne change pas
                    mvd = vd
                    tx2.setText('Vitesse = ' + "%.3f" % vd) # affiche la vitesse

# déplacement de la caméra                              
def spinCameraTask( task ):
    global px, py, angle, t0, t1,t2, cpt,t_rec
    t0 = int(time.time() * 1000) # temps de basse en millis secondes "milli()"
    mpx,mpy,ma = px,py,angle
    vd2 = vd/2000.0 # 2000.0 coef de vitesse entre réel et virtuel
    angle += ( servo_dir - 90 ) * s_servo_dir * vd2    # change l'angle en fonction du servo et de la vitesse de déplacement
    angleRadians = radians(angle)       # convertion des degré en radian
    px = px - sin(angleRadians) * vd2    # calcul de la prochaine position en x
    py = py + cos(angleRadians) * vd2    # calcul de la prochaine position en y
    
    x,y = int(px*94)+505 , int(py*94)+285
    if( collision[y,x]): px,py,angle = mpx,mpy,ma

    if (cam_fixe): # vue du haut
        camera.setPos(0, 0, 9)      # positionne la caméra
        camera.setHpr(0, -90 , 0 )  # rotation de la caméra  
    else: # vue de la voiture
        camera.setPos(px, py, pz)           # positionne la caméra
        camera.setHpr(angle, r_camera , 0)  # rotation de la caméra
        
    voiture.setPos(px, py, .0)    # positionne la caméra
    voiture.setHpr(angle, 0 , 0)  # rotation de la caméra
    
    if(save_on): # enregistrement trace
        
        if (t0>t1): # si la tempo 1 est finie
            if(px>3.446041572093964 and px<3.613385605812073 and py<-1.4547595977783203 and py>-2.305843412876129): # ligne bleu 
                save() # stop l'enrgistrement à la ligne bleu ( a modifier )
            t1 = t0 + t_rec # nouvelle ligne toutes les t_rec ms
            #tb.append( (px,py,angle) )
            ls.set_vertex(cpt, px , py, .01) # déplace le point à la position de la voiture
            cpt += 1 # prochain point
            if (cpt>lg): # si fin de la ligne qui compte "lg" points
                save() # stop l'enregistrement de la trace
            
    return Task.cont

# touches du clavier
def fd(): # flèche droite tourne à droite
    global servo_dir
    if (servo_dir>10): servo_dir -= 10
    tx1.setText('Direction = ' + "%.3f" % servo_dir) # affiche l'angle du servo)                    
    
def fg(): # flèche gauche tourne à gauche
    global servo_dir
    if (servo_dir<170): servo_dir += 10
    tx1.setText('Direction = ' + "%.3f" % servo_dir) # affiche l'angle du servo
    
def fh(): # flèche haut accélère
    global vd
    vd+=10
    tx2.setText('Vitesse = ' + "%.3f" % vd) # affiche la vitesse

def fb(): # flèche bas, décélère 
    global vd
    vd -=10
    tx2.setText('Vitesse = ' + "%.3f" % vd) # affiche la vitesse

def space(): # espace, stope le déplacement
    global px, py, angle, vd, servo_dir

    if (vd):    # 1er appuis sur espace
        vd = 0  # stop la vitesse
        tx2.setText('Vitesse = ' + "%.3f" % vd) # affiche la vitesse
    else:       # 2eme appuis sur espace init la position
        px = 3.5669
        py = -1.8663
        angle = 90 # l'angle ou va la voiture
        servo_dir = 90
        tx1.setText('Direction = ' + "%.3f" % servo_dir) # affiche l'angle du servo

def sortie(): # Echap ou q, fin du programme
    global start
    start = False
    base.destroy()
    ser.close()
    sys.exit()
    
def vsl1(): # slider rotation camera
    global r_camera
    r_camera = sl1['value'] # récupère la valeur du slider
    tsl1.setText('Rotation camera = ' + "%.3f" % r_camera) # affiche la valeur 
    
def vsl2(): # slider Hauteur camera
    global pz
    pz = sl2['value']  # récupère la valeur du slider
    tsl2.setText('Hauteur camera = ' + "%.3f" % pz) # affiche la valeur 

def vsl3(): # slider Focale camera
    global focale
    focale = sl3['value'] # récupère la valeur du slider
    tsl3.setText('Focale camera = ' + "%.3f" % focale) # affiche la valeur 
    base.camLens.setFov( focale ) # modifie la focale de la cam
    #tsl3.Fg = 1,0,1,1

def SkyB(): # bouton Skybox (environnement)
    global sky_on # état du bouton
    if(sky_on):
        sky_on = False
        skybox.hide() # désactive le SkyBox
        b1.setText(" SkyBox = Off ")
    else:
        sky_on = True
        skybox.show() # active le SkyBox
        b1.setText(" SkyBox = On  ")
        
def save(): # bouton sauvegarde
    global save_on, cpt
    
    if(save_on):
        save_on = False
        b2.setText("Sauvegarde = Off")
        #if cpt>500: cpt=500
        #for i in range(0,cpt):
        #    ls.set_vertex(i, tb[i][0] , tb[i][1], .3)
        #cpt -= 1
        #for i in range(cpt,500):
        #    ls.set_vertex(cpt, tb[cpt][0] , tb[cpt][1], .3)
        # cpt = 0
    else:
        save_on = True
        b2.setText("Sauvegarde = On")
        cpt = 0 # compteur pour les points de la nouvelle trace
        ls.set_vertex(0, 0.0, 0.0, -1000) # recule le 1er point pour masquer la ligne
        for i in range(0,lg): # création de lg points pour plus tard
            ls.set_vertex(i, 0, 0, -1000) # recule tous les points de la ligne pour la masquer

def trace(): # bouton Trace affiche ou pas la trace
    global trace_on # état du bouton
    if(trace_on):
        trace_on = False
        b3.setText("Trace = Off")
        pls.hide() # cache la trace
    else:  # interdire la lecture si pas de sauvegarde si tableau vide
        trace_on = True
        b3.setText("Trace = On")
        pls.show() # affiche la trace
        
def cam(): # bouton vue de haut
    global cam_fixe
    if(cam_fixe):
        cam_fixe = False
        b4.setText("Vue de Haut = Off")
        if (brouillard_on): brouillard.setExpDensity(brouillard_Density) # active le brouillard
        base.camLens.setFov( focale ) # positionne la focale normalement
        sl1['state'] = 1 # active le slider Rotation caméra
        sl2['state'] = 1 # active le slider Hauteur caméra
        sl3['state'] = 1 # active le slider Focale caméra
    else:
        cam_fixe = True
        b4.setText("Vue de Haut =  On")
        brouillard.setExpDensity(0) # desactive le brouillard
        base.camLens.setFov( 70 ) # positionne la focale à 70
        sl1['state'] = 0 # desactive le slider Rotation caméra
        sl2['state'] = 0 # desactive le slider Hauteur caméra
        sl3['state'] = 0 # desactive le slider Focale caméra
        
def brou(): # bouton brouillard
    global brouillard_on
    if(brouillard_on):
        brouillard_on = False
        brouillard.setExpDensity(0) # desactive le brouillard
        b5.setText(" Brouillard = Off ") # affiche le texte
    else:
        brouillard_on = True
        brouillard.setExpDensity(brouillard_Density) # active le brouillard
        b5.setText(" Brouillard =  On ") # affiche le texte

def brui(): # bouton bruit tache au sol
    global bruit_on
    
    if(bruit_on):
        bruit_on = False
        bruit.hide() # cache la tache
        b6.setText(" Bruit = Off ") # affiche le texte
    else:
        bruit_on = True
        bruit.show() # affiche la tache
        b6.setText(" Bruit =  On ") # affiche le texte

def souris_click():
    if (1): # affiche les coordonnées de la souris
        if souris.hasMouse(): # le pointeur est sur l'écran de Panda3D
            x,y = souris.getMouseX()*5.95, souris.getMouseY()*3.5 # coordonées de l'écran modifier pour s'adapter au sol 3D
            print ( x, y, " /", int(x*100)+487 , int(y*100)+245 ) #affiche la position de la souris corigé en foction de la vue du haut
            if (bruit_on and cam_fixe ):
                bruit.setPos(x, y, 0.001) # déplace la tache bruit
                
base = ShowBase() # active la scene panda3D

base.camLens.setNear(.1) # cliping de la caméra
base.taskMgr.add(spinCameraTask, "SpinCameraTask") # active la caméra

# test les touches
base.accept('arrow_right', fd)  #flèche droite tourne à droite
base.accept('arrow_left', fg)   #flèche gauche tourne à gauche
base.accept('arrow_up', fh)     #flèche haut accelère
base.accept('arrow_down', fb)   #flèche bas ralanti
base.accept('q', sortie )       # fin du programme
base.accept('s', save )         # enregistre le tracé
base.accept('v', cam )          # vue de haut
base.accept('escape', sortie )  # fin du programme
base.accept('space', space )    # stop et revient à la ligne bleu au départ

# chargement du décors, piste, plintes sol, mur du fond et du portique le tout en un seul objet
sol = base.loader.loadModel("maillage_3D/piste.gltf" ) # charge la piste
sol.reparentTo(base.render)

# chargement de la voiture
voiture = base.loader.loadModel("maillage_3D/voiture.gltf" ) # charge la voiture pour test de colision
voiture.reparentTo(base.render)

# chargement du skybox
skybox = base.loader.loadModel("maillage_3D/skybox_1024")
skybox.reparentTo(base.camera)
#skybox.set_two_sided(True)
skybox.set_bin("background", 0)
skybox.set_depth_write(False)
skybox.set_compass() # bonge en fonction de la cam

# le brouillard
brouillard = Fog("Fog Name")
brouillard.setColor(0.2, 0.2, 0.2)
brouillard.setExpDensity(brouillard_Density)
render.setFog(brouillard)

# bruit tache au sol
bruit = base.loader.loadModel("maillage_3D/bruit.gltf" ) # charge la grosse tache 
bruit.setPos(1.493697822, -1.7978323698, 0.01) # au dépard la tache est entre la ligne bleu et la rouge
bruit.reparentTo(render)
bruit.setColor(0.5, 0.2, 0.5) # la couleur de la grosse tache
bruit.hide() # au début on ne l'affiche pas

# ligne de visualisation du trajet "trace"
ls = LineSegs() # objet maillage ligne
ls.setThickness(10) # largeur de la trace
ls.setColor(1.0, 0.0, 0.0, 1.0) # couleur de la trace
ls.moveTo(0.0, 0.0, -1000)
for i in range(0,lg): # création de lg points pour plus tard
    ls.drawTo(0, 0, -1000)
pls = NodePath( ls.create(True) ) # patch de la ligne
pls.reparentTo(base.render)

# affichage du masque en 3D des collisions
if(0):
    ls2 = LineSegs() # objet maillage ligne
    ls2.setThickness(5) # largeur de la trace
    ls2.setColor(1.0, 0.0, 1.0, 0.0) # couleur de la trace
    for x in range(0,mx,10):
        for y in range(0,my,10): #mx,my
            x1,y1 = (x - 505)/94, (y - 285)/94 # mpx,mpy = 505,285
            if(collision[y,x]): ls2.moveTo(x1, y1, 0.01)
    pls2 = NodePath( ls2.create(True) ) # patch de la ligne
    pls2.reparentTo(base.render)


#souris pour les coordonnées en vue de dessus
souris = base.mouseWatcherNode
base.accept("mouse1",souris_click) # click gauche
#souris.taskMgr.add(update,"update")

# interface Sliders, boutons et textes, x et y = position des objets
# Slider rotation caméra + texte
x,y = -1.5, .95
sl1  = DirectSlider(range=(-90,90), value = r_camera, pageSize=3, pos=(x, 0,y)  , scale=0.3, command=vsl1)
tsl1 = OnscreenText(pos = (x+.32, y-.01), bg = (1,1,1,.8),  scale=0.035, align = TextNode.ALeft ) # bg = couleur de fond

# Slider hauteur camera + texte
x,y = -1.5, .9
sl2  = DirectSlider(range=(0,.5), value = pz, pageSize=3, pos=(x, 0,y)  , scale=0.3, command=vsl2)
tsl2 = OnscreenText(pos = (x+.32, y-.01), bg = (1,1,1,.8),  scale=0.035, align = TextNode.ALeft ) # bg = couleur de fond

# Slider focale caméra + texte
x,y = -1.5, .85
sl3  = DirectSlider(range=(1,120), value = focale, pageSize=3, pos=(x, 0,y)  , scale=0.3, command=vsl3)
tsl3 = OnscreenText(pos = (x+.32, y-.01), bg = (1,1,1,.8), scale=0.035, align = TextNode.ALeft ) # bg = couleur de fond

# Texte affichage direction
x,y = -0.85, .95
tx1 =  DirectLabel(pos = (x+.32,0, y-.01),text = "DIRECTION = XXXXXX",  scale=0.03 )

# Texte affichage vitesse
x,y = -0.85, .9
tx2 =  DirectLabel(pos = (x+.32,0, y-.01),text = "VITESSE = XXXXXXX",  scale=0.03 )

# bouton SkyBox
x,y = 0, .95
b1 = DirectButton(pos = (x+.32,0, y-.01), text=(".    SkyBox = On     ."), scale=.035, command=SkyB)

# bouton Sauvegarde
x,y = -.5, .95 #                                 ".XXXXXXXXXXXXXXXXXXXX."
b2 = DirectButton(pos = (x+.32,0, y-.01), text=(".  Sauvegarde = Off  ."), scale=.035, command=save)

# bouton trace
x,y = -.5, .9  #                               ".XXXXXXXXXXXXXXXXXXXX."
b3 = DirectButton(pos = (x+.32,0, y-.01), text=(".   Trace = On    ."), scale=.035, command=trace)

# bouton camera fixe
x,y = .5, .95 #                                ".XXXXXXXXXXXXXXXXXXXX."
b4 = DirectButton(pos = (x+.32,0, y-.01), text=(". Vue de Haut = Off  ."), scale=.035, command=cam)

# bouton brouillard
x,y = 0, .9
b5 = DirectButton(pos = (x+.32,0, y-.01), text=(".  Brouillard =  On  ."), scale=.035, command=brou)

# bouton bruit
x,y = 0, .85
b6 = DirectButton(pos = (x+.32,0, y-.01), text=(".  bruit =  Off  ."), scale=.035, command=brui)

# ouverture d'une tache de fond pour la lecture USB
th_serie = threading.Thread(target = serie) # prépare le Thread du port série
th_serie.start() # démarre le Thread pour le port série

run() # Démarre panda3D





