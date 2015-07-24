#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

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

RF24 radio(9,10);
int i = 0;
char payload[42];

//we only need a write pipe, but am planning to use it later
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL,0xF0F0F0F0D2LL };

void setup() {
   Serial.begin(9600); //Debug 
  printf_begin();
  radio.begin();
  radio.setChannel(0x4c);
  radio.setAutoAck(1);
  radio.setRetries(15,15);
  radio.setDataRate(RF24_250KBPS);
  radio.setPayloadSize(42);
  radio.openReadingPipe(1,pipes[0]);
  radio.openWritingPipe(pipes[1]);
  radio.startListening();
  radio.printDetails(); //for Debugging
  
  
}

void loop() {
  radio.stopListening();
  char SendPayload[42]  = "passage numero ";
  char b[5];
  dtostrf(i,2,2,b);
  strcat(SendPayload,b);
  bool ok = radio.write(&SendPayload,sizeof(SendPayload));
  radio.startListening(); 
   
  i++;
   unsigned long started_waiting_at = millis();
    bool timeout = false;
    while ( ! radio.available() && ! timeout )
      if (millis() - started_waiting_at > 700 )
        timeout = true;

    // Describe the results
    if ( timeout )
    {
      printf("Failed, response timed out.\n\r");
    }
    else
    {
      // Get the packet from the radio
     
      radio.read( &payload, sizeof(payload) );
    }
    Serial.println(payload);

  // slow down a bit
  delay(3000);

}
