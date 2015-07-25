#!/usr/bin/env python
# -*- coding: utf-8 -*-

# raspberry pi nrf24l01 hub

from nrf24 import NRF24
import time
from time import gmtime, strftime

pipes = [[0xf0, 0xf0, 0xf0, 0xf0, 0xe1], [0xf0, 0xf0, 0xf0, 0xf0, 0xd2]]

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

#radio.startListening()
#radio.stopListening()

#radio.printDetails()
radio.startListening()

def extract(raw_string, start_marker, end_marker):
    start = raw_string.index(start_marker) + len(start_marker)
    end = raw_string.index(end_marker, start)
    return raw_string[start:end]

print ("Attente réception du capteur")
while True:
    pipe = [0]
    while not radio.available(pipe, True):
        time.sleep(1000/1000000.0)
    recv_buffer = []
    radio.read(recv_buffer)
    out = ''.join(chr(i) for i in recv_buffer)
    
    capt=extract(out,'C','C')
    temper=extract(out,'T','T')
    humid=extract(out,'H','H')
    tens=extract(out,'V','V')

    capt = str(int(float(capt)))
    
    print ("Numéro du capteur : " + capt)
    print ("La temppérature : " + temper + "°C")
    print ("Niveau d'humidité : " + humid + "%")
    print ("Tension de l'accu : " + tens + " volts")
    print
    
"""    radio.stopListening();
    ok = radio.write(out);
    radio.startListening();
"""
