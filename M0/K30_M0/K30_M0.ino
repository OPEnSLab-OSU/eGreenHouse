/*
  Basic Arduino example for K-Series sensor
  Created by Jason Berger
  Co2meter.com  
*/

/*
// For Arduino Uno and 32u4 use softSerial
#include "SoftwareSerial.h"

SoftwareSerial K_30_Serial(10,11);  //Sets up a virtual serial port
                                    //Using pin 12 for Rx and pin 13 for Tx
*/
// for MO, create new UART SERCOM on Serial2
// See https://learn.adafruit.com/using-atsamd21-sercom-to-add-more-spi-i2c-serial-ports/creating-a-new-serial
#include <Arduino.h>   // required before wiring_private.h
#include "wiring_private.h" // pinPeripheral() function

// RX pin 11, TX pin 10
Uart Serial2 (&sercom1, 11, 10, SERCOM_RX_PAD_0, UART_TX_PAD_2);
void SERCOM1_Handler()
{
  Serial2.IrqHandler();
}

byte readCO2[] = {0xFE, 0X44, 0X00, 0X08, 0X02, 0X9F, 0X25};  //Command packet to read Co2 (see app note)
byte response[] = {0,0,0,0,0,0,0};  //create an array to store the response

//multiplier for value. default is 1. set to 3 for K-30 3% and 10 for K-33 ICB
int valMultiplier = 1;

void setup() 
{
  // put your setup code here, to run once:
  Serial.begin(9600);         //Opens the main serial port to communicate with the computer
  Serial2.begin(9600);
  // Assign pins 10 & 11 SERCOM functionality
  pinPeripheral(10, PIO_SERCOM);
  pinPeripheral(11, PIO_SERCOM);
  // K_30_Serial.begin(9600);    //Opens the virtual serial port with a baud of 9600
}

void loop() 
{
  sendRequest(readCO2);
  unsigned long valCO2 = getValue(response);
  Serial.print("Co2 ppm = ");
  Serial.println(valCO2);
  delay(2000);
  
}

void sendRequest(byte packet[])
{
  while(!Serial2.available())  //keep sending request until we start to get a response
  {
    Serial2.write(readCO2,7);
    delay(50);
  }
  
  int timeout=0;  //set a timeoute counter
  while(Serial2.available() < 7 ) //Wait to get a 7 byte response
  {
    timeout++;  
    if(timeout > 10)    //if it takes to long there was probably an error
      {
        while(Serial2.available())  //flush whatever we have
          Serial2.read();
          
          break;                        //exit and try again
      }
      delay(50);
  }
  
  for (int i=0; i < 7; i++)
  {
    response[i] = Serial2.read();
  }  
}

unsigned long getValue(byte packet[])
{
    int high = packet[3];                        //high byte for value is 4th byte in packet in the packet
    int low = packet[4];                         //low byte for value is 5th byte in the packet

  
    unsigned long val = high*256 + low;                //Combine high byte and low byte with this formula to get value
    return val* valMultiplier;
}


