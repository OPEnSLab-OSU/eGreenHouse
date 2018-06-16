// eGreenhouse Feather Proto M0 + Adalogger + nRF

//////// Adalogger SD /////////
/* analog sensors on analog ins 0, 1, and 2
  SD card attached to SPI bus as follows:
  MOSI - pin 11
  MISO - pin 12
  CLK - pin 13
  CS - pin 4 (for MKRZero SD: SDCARD_SS_PIN)
*/


#include <Arduino.h>
#include "wiring_private.h"
#include <Wire.h>
#include "Adafruit_SHT31.h" //for the SHT31-D sensors (temperature and hummidity)
#include <Adafruit_Sensor.h> // for the TSL2561 sensor(luminosity sensor)
#include <SPI.h>
#include <SD.h>
#include <Adafruit_TSL2561_U.h> // for the TSL2561 sensor(luminosity sensor)
#include "RTClib.h" // for the RTC
RTC_DS3231 rtc;

// for MO, create new UART SERCOM on Serial2
// See https://learn.adafruit.com/using-atsamd21-sercom-to-add-more-spi-i2c-serial-ports/creating-a-new-serial
#include <Arduino.h>   // required before wiring_private.h
#include "wiring_private.h" // pinPeripheral() function

//// NRF setup ////
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include <OSCBundle.h>

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x) //to concatenate a predefined number to a string literal, use STR(x)

#define FAMILY "/LOOM"
#define DEVICE "/Ishield"
#define INSTANCE_NUM 0  // Unique instance number for this device, useful when using more than one of the same device type in same space

#define IDString FAMILY DEVICE STR(INSTANCE_NUM) // C interprets subsequent string literals as concatenation: "/Loom" "/Ishield" "0" becomes "/Loom/Ishield0"

RF24 radio(5,6);                    // nRF24L01(+) radio attached using Getting Started board 

RF24Network network(radio);          // Network uses that radio

const uint16_t this_node = 00;        // Address of our node in Octal format
const uint16_t other_node = 01;       // Address of the other node in Octal format

const unsigned long interval = 5000; //ms  // How often to send 'hello world to the other unit

unsigned long last_sent;             // When did we last send?
unsigned long packets_sent;          // How many have we sent already


// RX pin 11, TX pin 10
Uart Serial2 (&sercom1, 12, 11, SERCOM_RX_PAD_3, UART_TX_PAD_0);
void SERCOM1_Handler()
{
  Serial2.IrqHandler();
}

byte readCO2[] = {0xFE, 0X44, 0X00, 0X08, 0X02, 0X9F, 0X25};  //Command packet to read Co2 (see app note)
byte response[] = {0,0,0,0,0,0,0};  //create an array to store the response

//multiplier for value. default is 1. set to 3 for K-30 3% and 10 for K-33 ICB
int valMultiplier = 1;

const int chipSelect = 10;  // chip select for adalogger

///// configurion section for the the luminosity sensor//////
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_LOW, 12345);
void configureSensor(void)
{
  /* You can manually set the gain or enable auto-gain support */
  //tsl.setGain(TSL2561_GAIN_1X);      /* No gain ... use in bright light to avoid sensor saturation */
  //tsl.setGain(TSL2561_GAIN_16X);     /* 16x gain ... use in low light to boost sensitivity */
  tsl.enableAutoRange(true);            /* Auto-gain ... switches automatically between 1x and 16x */

  /* Changing the integration time gives you better sensor resolution (402ms = 16-bit data) */
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);      /* fast but low resolution */
  //tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);  /* medium resolution and speed   */
  //  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);  /* 16-bit data but slowest conversions */

  /* Update these values depending on what you've set above! */
  Serial.print  (F("Gain:         ")); Serial.println(F("Auto"));
  Serial.print  (F("Timing:       ")); Serial.println(F("13 ms"));
}

void setup() {
 
  // Open serial communications and wait for port to open:
  Serial.begin(9600);         //Opens the main serial port to communicate with the computer
  Serial2.begin(9600);
  // Assign pins 10 & 11 SERCOM functionality
  pinPeripheral(11, PIO_SERCOM);
  pinPeripheral(12, PIO_SERCOM);
  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  SPI.begin();
  radio.begin();
  network.begin(/*channel*/ 90, /*node address*/ this_node);
  
if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
}
  Serial.print("Initializing SD card...");
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);

  }
  Serial.println("card initialized.");
  
}

void loop() {
  
  DateTime now = rtc.now();
  
  sensors_event_t event; //get the luminosity data
  tsl.getEvent(&event);

  sendRequest(readCO2);
  
  unsigned long valCO2 = getValue(response);
  
  // open the file. note that only one file can be open at a time, so you have to close this one before opening another.
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  // if the file is available, write to it:
  if (dataFile) {
    dataFile.print(now.day(), DEC);
    dataFile.print(',');
    dataFile.print(now.month(), DEC);
    dataFile.print(',');
    dataFile.print(now.year(), DEC);
    dataFile.print(',');
    dataFile.print(now.hour(), DEC);
    dataFile.print(',');
    dataFile.print(now.minute(), DEC);
    dataFile.print(',');
    dataFile.print(now.second(), DEC);
    dataFile.print(',');
    dataFile.print(event.light);//[lux]
    dataFile.print(',');
    dataFile.print(valCO2);
    dataFile.close();
    
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println(F("error opening datalog.txt"));
  }

  /////// print to the serial port too: ////////
  Serial.print(now.day(), DEC);
  Serial.print(',');
  Serial.print(now.month(), DEC);
  Serial.print(',');
  Serial.print(now.year(), DEC);
  Serial.print(',');
  Serial.print(now.hour(), DEC);
  Serial.print(',');
  Serial.print(now.minute(), DEC);
  Serial.print(',');
  Serial.print(now.second(), DEC);
  Serial.print(',');
  Serial.print(event.light);
  Serial.print(',');
  Serial.print(valCO2);
  Serial.println();

  network.update();              // Check the network regularly

    Serial.print("Sending...");

    OSCBundle bndl;
    bndl.add("addr").add((unsigned long) valCO2);

    char message[300];

    memset(message, '\0', 300);

    get_OSC_string(&bndl, message);
    //char msg[120];
    //strcpy(msg, message);

    print_bundle(&bndl);
    
    RF24NetworkHeader header(/*to node*/ other_node);
    bool ok = network.write(header,message,strlen(message));
    if (ok)
      Serial.println("ok.");
    else
      Serial.println("failed.");
  
  delay (1000);// delay the logging time for x ms

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

