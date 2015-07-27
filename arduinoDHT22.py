#!/usr/bin/env python
# -*- coding: utf-8 -*-

from nrf24 import NRF24
import time
from time import gmtime, strftime
import MySQLdb
import xml.dom.minidom

"""import données connexion SGL d'un fichier config en XML"""
tree = xml.dom.minidom.parse("/home/pi/nrf24pihub/configSQL.xml")
valeurListe = tree.getElementsByTagName("SQL")

"""Variable de connexion R24"""
pipes = [[0xf0, 0xf0, 0xf0, 0xf0, 0xe1], [0xf0, 0xf0, 0xf0, 0xf0, 0xd2]]

"""Initialisation connexion R24"""
radio = NRF24()
radio.begin(0, 0,25,18) #set gpio 25 as CE pin
radio.setRetries(15,15)
radio.setPayloadSize(52)
radio.setChannel(0x4c)
radio.setDataRate(NRF24.BR_250KBPS)
radio.setPALevel(NRF24.PA_MAX)
radio.setAutoAck(1)
radio.openWritingPipe(pipes[0])
radio.openReadingPipe(1, pipes[1])

radio.startListening()
"""fin initialisation R24"""

"""Fonction pour extraire de la variable receptionnée les différentes valeurs"""
def extract(raw_string, start_marker, end_marker):
    start = raw_string.index(start_marker) + len(start_marker)
    end = raw_string.index(end_marker, start)
    return raw_string[start:end]

print
print ("Attente réception du capteur")
print
"""Boucle infinie de réception des données"""
while True:
    pipe = [0]
    """Si pas de réception on attends ..."""
    while not radio.available(pipe, True):
        time.sleep(1000/1000000.0)

    recv_buffer = []
    radio.read(recv_buffer) #Les données réceptionnées sont mise dans la variable
    out = ''.join(chr(i) for i in recv_buffer)#création d'une variable blobage avec les valeurs du tableu buffer

    """Création de la date et heure en francais et en SQL"""
    now = time.localtime(time.time())  
    year, month, day, hour, minute, second, weekday, yearday, daylight = now
    dateheure = "%02d/%02d/%04d" % (day, month, year) + "  " + "%02d:%02d:%02d" % (hour, minute, second)
    dateheureSQL = "%04d-%02d-%02d" % (year, month, day) + " " + "%02d:%02d:%02d" % (hour, minute, second)

    print dateheure
    
    """"La variable out est décortiquée avec les données de numéro de capteur, température, humidité, tension pile """
    capt=extract(out,'C','C')
    temper=extract(out,'T','T')
    humid=extract(out,'H','H')
    tens=extract(out,'V','V')

    capt = str(int(float(capt)))#on transforme le numéro de capteur d'abord en float puis en int pour finir en str
    """affichage des données recues pour es teste"""
    print ("Numéro du capteur : " + capt)
    print ("La temppérature   : " + temper + "°C")
    print ("Niveau d'humidité : " + humid + "%")
    print ("Tension de l'accu : " + tens + " volts")
    print

    """Connexion et insertion de la données dans la base"""
    for valeur in valeurListe:
    	#connexion  à la base de données
    	db = MySQLdb.connect(valeur.attributes['ip'].value, valeur.attributes['login'].value, valeur.attributes['mdp'].value, valeur.attributes['dbase'].value)
  	dbSQL = db.cursor()
    sql = "INSERT INTO `Capteurs_Temp_Portable`(`Date_Heure`, `Temperature`, `Humidite`, `Tension_Pile`, `N_Capteur`)\
    	VALUES ('" + dateheureSQL + "', " + temper + ", " + humid + ", " + tens + ", " + capt + ")"

    print sql
    dbSQL.execute(sql)
    db.commit()

    db.close()
    
"""    radio.stopListening();
    ok = radio.write(out);
    radio.startListening();
"""
