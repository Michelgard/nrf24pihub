#include <DHT.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <SPI.h>
#include "nRF24L01.h"
#include <RF24.h>

#define DHTPIN 6 // entrée capteur dht22
#define DHTTYPE DHT22
#define DHT22_POWER 3 //alimentation du capteur DHT22

#define PIN_PILE 1 // entrée analogique

DHT dht(DHTPIN, DHTTYPE);

//variables radio
RF24 radio(9,10); //création instance radio (pin ce,csn)
float txbuffer[4] = {1, 0, 0, 0}; // l’indice 0 du tableau représente le N° du themometre ici le 1
//variable température
float temperature;
float humidite;
//variables tension
const float coeff_division = 2.0; // constante pont diviseur
float real_bat = 0;
unsigned int raw_bat;
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL,0xF0F0F0F0D2LL };

// nrf24 debug
int serial_putc( char c, FILE * ) 
{
  Serial.write( c );
  return c;
} 
void printf_begin(void)
{
  fdevopen( &serial_putc, 0 );
} 

ISR (WDT_vect)
{
wdt_disable(); //désactive le watchdog
}

void mywatchdogenable()
{
MCUSR = 0;
WDTCSR = _BV (WDCE) | _BV (WDE);
WDTCSR = _BV (WDIE) | _BV (WDP3) | _BV (WDP0); //délai de 8 secondes
wdt_reset();
//ADCSRA = 0; //désactive ADC
set_sleep_mode (SLEEP_MODE_PWR_DOWN);
sleep_enable();
MCUCR = _BV (BODS) | _BV (BODSE);
MCUCR = _BV (BODS);
sleep_cpu();
sleep_disable();
}

void setup()
{
  printf_begin();
  //Serial.begin(9600); //Debug 
dht.begin();

pinMode(DHT22_POWER, OUTPUT);

pinMode(PIN_PILE,INPUT);

radio.begin();
  radio.setChannel(0x4c);
  radio.setAutoAck(1);
  radio.setRetries(15,15);
  radio.setDataRate(RF24_250KBPS);
  radio.setPayloadSize(52);
  radio.openReadingPipe(1,pipes[0]);
  radio.openWritingPipe(pipes[1]);
  radio.stopListening();
  //radio.printDetails(); //for Debugging
}

void loop ()
{
char SendPayload[52]  = "RES :";
char tempstr[10] = "";
digitalWrite(DHT22_POWER, HIGH); //alimente le capteur DHT22
delay(1000);
txbuffer[1] = dht.readTemperature();
txbuffer[2] = dht.readHumidity();
delay(100);
digitalWrite(DHT22_POWER, LOW); //alimente le capteur DHT22


/* Mesure de la tension brute */
raw_bat = analogRead(PIN_PILE);
/* Calcul de la tension réel */
txbuffer[3] = ((raw_bat * (3.3 / 1024)) * coeff_division);

 strcat(SendPayload, "C");
 dtostrf(txbuffer[0],2,2,tempstr); 
 strcat(SendPayload,tempstr); 
 strcat(SendPayload, "C"); 

strcat(SendPayload, "T");
 dtostrf(txbuffer[1],2,2,tempstr); 
 strcat(SendPayload,tempstr); 
 strcat(SendPayload, "T"); 
 
 strcat(SendPayload, "H");
 dtostrf(txbuffer[2],2,2,tempstr); 
 strcat(SendPayload,tempstr); 
 strcat(SendPayload, "H");
 
 strcat(SendPayload, "V");
 dtostrf(txbuffer[3],2,2,tempstr); 
 strcat(SendPayload,tempstr); 
 strcat(SendPayload, "V");

radio.powerUp(); //alimente le module nrf24l01+
delay(1000);
 bool ok = radio.write(&SendPayload,sizeof(SendPayload));
delay(1000);
radio.powerDown(); //arrêt de l’alimentation du module nrf24l01+

for (int i=0; i < 35; i++) //mise en veille pendant ~~ 5mn secondes
mywatchdogenable();
}
