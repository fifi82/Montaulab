
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
img = iio.imread("dependance/textures/masque.png")
gy,gx = img.shape # taille de la grille de collision
collision = np.zeros((gy, gx), dtype=np.bool) # c'éation d'un tableau booleen vide

for x in range(0,gx): # transforme l'image masque en tableau booleen
    for y in range(0,gy):
        collision[gy-y-1,x] = img[y,x] > 127       # gy-y-1 inverse l'image au chargement

gpx,gpy,gzz = 505,285,94 # pour transformation datas réelles en data tableau bool 

cam_fixe = False # active la vue de haut

brouillard_Density = 0.2 # opacité du brouillard

bruit_on = False # affiche ou pas la tache
sky_on = True # affiche ou pas le SkyBox, l'environement
save_on = False # active la sauvegarde du trajet
trace_on = True # active l'affichage de la trace sauvegardée
menu_on = True # affiche ou pas le menu
grille_on = False # affiche la grille de colisions

t_rec = 200 # enregistre la position toutes les n millis seconde

t0,t1,t2,t3 = 0,0,0,0 # tempos , t1 temps pour engegistrement, t2 temps au tour, t3 inib ligne arivée

nb_tour = 1 # nombre de tour à faire
n_tour = 0 # compteur de tour fait

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
                    tx[0].setText('Direction = ' + "%.3f" % servo_dir) # affiche l'angle du servo
                if mvd != vd: # évite de réafficher si la valeur ne change pas
                    mvd = vd
                    tx[1].setText('Vitesse = ' + "%.3f" % vd) # affiche la vitesse

# déplacement de la caméra                              
def spinCameraTask( task ):
    global vd,px, py, angle, t0, t1,t2, t3, cpt,t_rec, n_tour
    t0 = int(time.time() * 1000) # temps de basse en millis secondes "millis()"
    mpx,mpy,ma = px,py,angle
    vd2 = vd/2000.0 # 2000.0 coef de vitesse entre réel et virtuel
    angle += ( servo_dir - 90 ) * s_servo_dir * vd2    # change l'angle en fonction du servo et de la vitesse de déplacement
    angleRadians = radians(angle)       # convertion des degré en radian
    px = px - sin(angleRadians) * vd2    # calcul de la prochaine position en x
    py = py + cos(angleRadians) * vd2    # calcul de la prochaine position en y
    
    x,y = int(px*gzz)+gpx , int(py*gzz)+gpy
    if( collision[y,x] ):
        px,py,angle,vd = mpx,mpy,ma,0 # bloque la voiture en cas de collision
        boom.play()

    if (cam_fixe): # vue du haut
        camera.setPos(0, 0, 9)      # positionne la caméra
        camera.setHpr(0, -90 , 0 )  # rotation de la caméra  
    else: # vue de la voiture
        camera.setPos(px, py, pz)           # positionne la caméra
        camera.setHpr(angle, r_camera , 0)  # rotation de la caméra
        
    voiture.setPos(px, py, 0.01)    # positionne la caméra
    voiture.setHpr(angle, 0 , 0)  # rotation de la caméra
    
    if(save_on): # enregistrement trace (bouge les points de la ligne déjà créée)
        
        if (t0>t1): # si la tempo 1 est finie
            #if(px>3.446041572093964 and px<3.613385605812073 and py<-1.4547595977783203 and py>-2.305843412876129): # ligne bleu 
            #    save() # stop l'enrgistrement à la ligne bleu ( a modifier )
            t1 = t0 + t_rec # nouvelle ligne toutes les t_rec ms
            #tb.append( (px,py,angle) )
            ls.set_vertex(cpt, px , py, .01) # déplace le point à la position de la voiture
            cpt += 1 # prochain point
            if (cpt>lg): # si fin de la ligne qui compte "lg" points
                save_b1() # stop l'enregistrement de la trace
            
        if (px<-2.487187474966049 and px>-3.084374874830246 and py<-1.289820909500122 and py>-2.378416419029236):
            if (time.time() > t3):
                t3 = time.time() + 10
                n_tour -=1;
                
            if (not n_tour):
                save_b1()
                vd = 0
                t2 = time.time()- t2    
                print("%.2f" % t2,"s")
            
    return Task.cont

# touches du clavier
def fd(): # flèche droite tourne à droite
    global servo_dir
    if (servo_dir>10): servo_dir -= 10
    tx[0].setText('Direction = ' + "%.0f" % servo_dir + "°" ) # affiche l'angle du servo)                    
    
def fg(): # flèche gauche tourne à gauche
    global servo_dir
    if (servo_dir<170): servo_dir += 10
    tx[0].setText('Direction = ' + "%.0f" % servo_dir + "°" ) # affiche l'angle du servo
    
def fh(): # flèche haut accélère
    global vd
    vd+=10
    tx[1].setText('Vitesse = ' + "%.0f" % vd + "Km/h") # affiche la vitesse

def fb(): # flèche bas, décélère 
    global vd
    vd -=10
    tx[1].setText('Vitesse = ' + "%.0f" % vd + "Km/h") # affiche la vitesse

def space(): # espace, stope le déplacement
    global px, py, angle, vd, servo_dir

    if (vd):    # 1er appuis sur espace
        vd = 0  # stop la vitesse
        tx[1].setText('Vitesse = ' + "%.3f" % vd) # affiche la vitesse
    else:       # 2eme appuis sur espace init la position
        px = 3.5669
        py = -1.8663
        angle = 90 # l'angle ou va la voiture
        servo_dir = 90
        tx[0].setText('Direction = ' + "%.3f" % servo_dir) # affiche l'angle du servo

def sortie(): # Echap ou q, fin du programme
    global start
    start = False
    ser.close()
    base.destroy()
    time.sleep(1)
    sys.exit()    
    
    
def vsl0(): # slider rotation camera
    global r_camera
    r_camera = sl[0]['value'] # récupère la valeur du slider
    tsl[0].setText('Rotation camera = ' + "%.2f" % r_camera) # affiche la valeur 
    
def vsl1(): # slider Hauteur camera
    global pz
    pz = sl[1]['value']  # récupère la valeur du slider
    tsl[1].setText('Hauteur camera = ' + "%.2f" % pz) # affiche la valeur 

def vsl2(): # slider Focale camera
    global focale
    focale = sl[2]['value'] # récupère la valeur du slider
    tsl[2].setText('Focale camera = ' + "%.2f" % focale ) # affiche la valeur 
    base.camLens.setFov( focale ) # modifie la focale de la cam
    #tsl3.Fg = 1,0,1,1

def vsl3(): # slider épaisseur du brouillard
    global brouillard_Density
    brouillard_Density = sl[3]['value']  # récupère la valeur du slider
    brouillard.setExpDensity(brouillard_Density)  
    tsl[3].setText('Épaisseur brouillard = ' + "%.0f" % (100*brouillard_Density) + "%") # affiche la valeur 
    
def SkyB_b0(): # bouton Skybox (environnement)
    global sky_on # état du bouton
    sky_on = not sky_on
    if(sky_on):
        skybox.show() # active le SkyBox
        b[0].setText(" SkyBox = On  ")
    else:
        skybox.hide() # désactive le SkyBox
        b[0].setText(" SkyBox = Off ")
        
def save_b1(): # bouton sauvegarde
    global save_on, cpt,t2, n_tour

    save_on = not save_on
    if(save_on):
        b[1].setText("(S)auvegarde = On")
        cpt = 0 # compteur pour les points de la nouvelle trace
        ls.set_vertex(0, 0.0, 0.0, -1000) # recule le 1er point pour masquer la ligne
        for i in range(0,lg): # création de lg points pour plus tard
            ls.set_vertex(i, 0, 0, -1000) # recule tous les points de la ligne pour la masquer
        t2 = time.time()
        n_tour = nb_tour + 1
    else:
        b[1].setText("(S)auvegarde = Off")
        #if cpt>500: cpt=500
        #for i in range(0,cpt):
        #    ls.set_vertex(i, tb[i][0] , tb[i][1], .3)
        #cpt -= 1
        #for i in range(cpt,500):
        #    ls.set_vertex(cpt, tb[cpt][0] , tb[cpt][1], .3)
        # cpt = 0
        
def trace_b2(): # bouton Trace affiche ou pas la trace
    global trace_on # état du bouton
    trace_on = not trace_on
    if(trace_on):
        b[2].setText("(T)race = On")
        pls.show() # affiche la trace
    else:  # interdire la lecture si pas de sauvegarde si tableau vide
        b[2].setText("(T)race = Off")
        pls.hide() # cache la trace

        
def cam_b3(): # bouton vue de haut
    global cam_fixe
    cam_fixe = not cam_fixe
    if(cam_fixe):
        b[3].setText("(V)ue de Haut =  On")
        brouillard.setExpDensity(0) # desactive le brouillard
        base.camLens.setFov( 70 ) # positionne la focale à 70
        for i in sl: i['state'] = 0 # desactive le slider Rotation caméra
    else:
        b[3].setText("(V)ue de Haut = Off")
        brouillard.setExpDensity(brouillard_Density) # active le brouillard
        base.camLens.setFov( focale ) # positionne la focale normalement
        for i in sl: i['state'] = 1 # desactive le slider Rotation caméra
        
def bruit_b4(): # bouton bruit tache au sol
    global bruit_on
    bruit_on = not bruit_on
    if(bruit_on):
        bruit.show() # affiche la tache
        b[4].setText(" Bruit =  On ") # affiche le texte
    else:
        bruit.hide() # cache la tache
        b[4].setText(" Bruit = Off ") # affiche le texte
        
def grille_b5(): # bouton bruit tache au sol
    global grille_on
    grille_on = not grille_on
    if(grille_on):
        pls2.show() # affiche la grille
        b[5].setText(" Grills de collisions =  On ") # affiche le texte
    else:
        pls2.hide() # cache la grille
        b[5].setText(" Grille de collisions = Off ") # affiche le texte
        
        
def souris_click():
    global px,py
    if (cam_fixe): # affiche les coordonnées de la souris
        if souris.hasMouse(): # le pointeur est sur l'écran de Panda3D
            x,y = souris.getMouseX()*6.3, souris.getMouseY()*3.5 # coordonées de l'écran modifier pour s'adapter au sol 3D
            tx,ty = int(x*gzz)+gpx , int(y*gzz)+gpy # coordonnées du tableau de collisions
            print ( x, y, " /", tx , ty ) #affiche la position de la souris corigé en foction de la vue du haut
            #print ( x, y, " /", tx , ty, "/ ", collision[ty,tx] ) #affiche la position de la souris corigé en foction de la vue du haut

            if (bruit_on):
                bruit.setPos(x, y, 0.01) # déplace la tache bruit
            else:
                px,py = x, y
                
def Menu():
    global menu_on
    menu_on = not menu_on
    if(menu_on):
        for i in range( 0, len(menu) ): menu[i].show()
    else:
        for i in range( 0, len(menu) ): menu[i].hide()

props = WindowProperties( )
props.setTitle( 'Montaulab - fifi82' )                
base = ShowBase() # active la scene panda3D
base.win.requestProperties( props )
base.camLens.setNear(.1) # cliping de la caméra
base.taskMgr.add(spinCameraTask, "SpinCameraTask") # active la caméra

# test les touches
base.accept('arrow_right', fd)  #flèche droite tourne à droite
base.accept('arrow_left', fg)   #flèche gauche tourne à gauche
base.accept('arrow_up', fh)     #flèche haut accelère
base.accept('arrow_down', fb)   #flèche bas ralanti
base.accept('s', save_b1 )      # enregistre le tracé
base.accept('v', cam_b3 )       # vue de haut
base.accept('b', bruit_b4 )     # bruit tache au sol
base.accept('m', Menu )         # affiche ou pas le menu
base.accept('t', trace_b2 )     # affiche ou pas la trace
# base.accept('escape', sortie ) # fin du programme
base.accept('space', space )    # stop et revient à la ligne bleu au départ

# chargement du décors, piste, plintes sol, mur du fond et du portique le tout en un seul objet
sol = base.loader.loadModel("dependance/piste.gltf" ) # charge la piste
sol.reparentTo(base.render)

# chargement de la voiture
voiture = base.loader.loadModel("dependance/voiture.gltf" ) # charge la voiture pour test de colision
#voiture = base.loader.loadModel("dependance/fleche.gltf" ) # charge la voiture pour test de colision
voiture.reparentTo(base.render)

boom = base.loader.loadSfx("dependance/boom.ogg")

# chargement du skybox
skybox = base.loader.loadModel("dependance/skybox_1024")
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
bruit = base.loader.loadModel("dependance/bruit.gltf" ) # charge la grosse tache 
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
if(1):
    ls2 = LineSegs() # objet maillage ligne
    ls2.setThickness(5) # largeur de la trace
    ls2.setColor(1.0, 0.0, 1.0, 0.0) # couleur de la trace
    for x in range(0,gx,10):
        for y in range(0,gy,10): #mx,my
            x1,y1 = (x - gpx)/gzz, (y - gpy)/gzz # gpx,gpy,gzz = 505,285,94
            if(collision[y,x]): ls2.setColor(1.0, 1.0, 1.0, 0.0)
            else : ls2.setColor(0.0, 0.5, 0.0, 1)
            ls2.moveTo(x1, (y - gpy)/gzz , 0.01)
    pls2 = NodePath( ls2.create(True) ) # patch de la ligne
    pls2.reparentTo(base.render)
    pls2.hide()


#souris pour les coordonnées en vue de dessus
souris = base.mouseWatcherNode
base.accept("mouse1",souris_click) # click gauche

# interface Sliders, boutons et textes, x et y = position des objets
menu,sl,tsl,b,tx = [],[],[],[],[] # tableaux pour l'interface

# Slider 0: rotation caméra + texte
x,y = -1.5, .95
sl.append( DirectSlider(range=(-90,90), value = r_camera, pageSize=3, pos=(x, 0,y)  , scale=0.3, command=vsl0) )
tsl.append( OnscreenText(pos = (x+.32, y-.01), bg = (1,1,1,.8),  scale=0.035, align = TextNode.ALeft ) ) # bg = couleur de fond

# Slider 1: hauteur camera + texte
x,y = -1.5, .9
sl.append( DirectSlider(range=(0,.5), value = pz, pageSize=3, pos=(x, 0,y)  , scale=0.3, command=vsl1) )
tsl.append( OnscreenText(pos = (x+.32, y-.01), bg = (1,1,1,.8),  scale=0.035, align = TextNode.ALeft ) ) # bg = couleur de fond

# Slider 2: focale caméra + texte
x,y = -1.5, .85
sl.append( DirectSlider(range=(1,120), value = focale, pageSize=3, pos=(x, 0,y)  , scale=0.3, command=vsl2) )
tsl.append( OnscreenText(pos = (x+.32, y-.01), bg = (1,1,1,.8), scale=0.035, align = TextNode.ALeft ) ) # bg = couleur de fond

# Slider 3: épaisseur du brouillard
x,y = -1.5, .80
sl.append( DirectSlider(range=(0,1), value = brouillard_Density, pageSize=3, pos=(x, 0,y)  , scale=0.3, command=vsl3) )
tsl.append( OnscreenText(pos = (x+.32, y-.01), bg = (1,1,1,.8), scale=0.035, align = TextNode.ALeft ) )# bg = couleur de fond

# Texte affichage direction
x,y = -0.85, .95
tx.append( DirectLabel(pos = (x+.32,0, y-.01),text = "DIRECTION = XXXXXX",  scale=0.03 ) ) # tx[0]

# Texte affichage vitesse
x,y = -0.85, .9
tx.append( DirectLabel(pos = (x+.32,0, y-.01),text = "VITESSE = XXXXXXX",  scale=0.03 ) ) #tx[1]

# bouton 0: SkyBox
x,y = 0, .95
b.append( DirectButton(pos = (x+.32,0, y-.01), text=(".    SkyBox = On     ."), scale=.035, command=SkyB_b0) )

# bouton 1: Sauvegarde
x,y = -.5, .95 #                                 ".XXXXXXXXXXXXXXXXXXXX."
b.append( DirectButton(pos = (x+.32,0, y-.01), text=("(S)auvegarde = Off"), scale=.035, command=save_b1) )

# bouton 2: trace
x,y = -.55, .9  #                               ".XXXXXXXXXXXXXXXXXXXX."
b.append( DirectButton(pos = (x+.32,0, y-.01), text=("(T)race = On"), scale=.035, command=trace_b2) )

# bouton 3: camera fixe vue de haut
x,y = .5, .95 #                                ".XXXXXXXXXXXXXXXXXXXX."
b.append( DirectButton(pos = (x+.32,0, y-.01), text=("(V)ue de Haut = Off"), scale=.035, command=cam_b3) )

# bouton 4: bruit
x,y = 0, .9
b.append( DirectButton(pos = (x+.32,0, y-.01), text=("(B)ruit =  Off"), scale=.035, command=bruit_b4) )

# bouton 5: grille de collision
x,y = 0.45, .9
b.append( DirectButton(pos = (x+.32,0, y-.01), text=("Grille de collisions =  Off"), scale=.035, command=grille_b5) )

for i in range(0, len(b)): menu.append( b[i] ) # ajoute les boutons au menu

#aide
x,y = .7, .95
t =  "                           Aide\n"
t += "  Flèches haut et bas : avance/recul\n"
t += "  Flèches doite/gauche : direction\n"
t += "  Espace : Stop/Reset position\n"
t += "  M : affiche ou pas le (M)enu\n"
t += "_____________________\n"
menu.append( OnscreenText(pos = (x+.32, y-.01), bg = (1,1,1,.8),text=(t), scale=0.035, align = TextNode.ALeft ) ) # bg = couleur de fond

menu.extend( sl )  # ajoute les slider au menu
menu.extend( tsl ) # ajoute les textes des slider au menu
menu.extend( tx )  # ajoute les textes (direction et vitesse) au menu
menu.extend( b )   # ajoute les boutons au menu


# ouverture d'une tache de fond pour la lecture USB
th_serie = threading.Thread(target = serie) # prépare le Thread du port série
th_serie.start() # démarre le Thread pour le port série

run() # Démarre panda3D

print(" fin ")



