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

// for MO, create new UART SERCOM on Serial2
// See https://learn.adafruit.com/using-atsamd21-sercom-to-add-more-spi-i2c-serial-ports/creating-a-new-serial
#include <Arduino.h>   // required before wiring_private.h
#include "wiring_private.h" // pinPeripheral() function

#include <RF24Network.h>
#include <RF24.h>
#include <OSCBundle.h>
#include "RTClib.h" // for the RTC
RTC_DS3231 rtc;

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x) //to concatenate a predefined number to a string literal, use STR(x)

#define FAMILY "/LOOM"
#define DEVICE "/Ishield"
#define INSTANCE_NUM 0  // Unique instance number for this device, useful when using more than one of the same device type in same space

#define IDString FAMILY DEVICE STR(INSTANCE_NUM) // C interprets subsequent string literals as concatenation: "/Loom" "/Ishield" "0" becomes "/Loom/Ishield0"

//// define nRF communication ////
RF24 radio(5,6);                    // nRF24L01(+) radio attached using Getting Started board 
RF24Network network(radio);          // Network uses that radio
const uint16_t this_node = 01;        // Address of our node in Octal format
const uint16_t other_node = 00;       // Address of the other node in Octal format
const unsigned long interval = 5000; //ms  // How often to send 'hello world to the other unit
unsigned long last_sent;             // When did we last send?
unsigned long packets_sent;          // How many have we sent already
char message[300];

// Create Serial SERCOM for CO2 sensor: RX pin 12, TX pin 11
Uart Serial2 (&sercom1, 12, 11, SERCOM_RX_PAD_3, UART_TX_PAD_0);
void SERCOM1_Handler()
{
  Serial2.IrqHandler();
}

byte readCO2[] = {0xFE, 0X44, 0X00, 0X08, 0X02, 0X9F, 0X25};  //Command packet to read Co2 (see app note)
byte response[] = {0,0,0,0,0,0,0};  //create an array to store the response
//multiplier for value. default is 1. set to 3 for K-30 3% and 10 for K-33 ICB
int valMultiplier = 1;

const int chipSelect = 4;  // chip select for adalogger

// define temp/RH //
Adafruit_SHT31 sht31 = Adafruit_SHT31();   
//float t = 0; //temperature
//float h = 0; //relative humidity

// define sensor variables //
unsigned long valCO2 ;
sensors_event_t event; 
float t = 0;
float h = 0;
float dust = 0;

///// configurion section for the the luminosity sensor//////
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);
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
  Serial2.begin(9600);        //Opens serial port for the K-30 CO2 
  // Assign pins 10 & 11 SERCOM functionality
  pinPeripheral(11, PIO_SERCOM);
  pinPeripheral(12, PIO_SERCOM);
  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
}
if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
      
}
  Serial.print("Initializing SD card...");
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);

  }
  Serial.println("card initialized.");

  Serial.println(F("Initializing SHT31"));
  if (! sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
    Serial.println(F("Couldn't find SHT31"));
  while (1) delay(1);
  }
  
  Serial.println("...SHT31 initialized");
  
  SPI.begin();
  radio.begin();
  network.begin(/*channel*/ 90, /*node address*/ this_node);
}

void loop() {

//// nRF receive from HUB ////

  while(network.available() == 0 ){
    network.update();// update the netowrk until we have something
    Serial.println("Waiting to receive something from hub.");
    delay(500);
  }

  while ( network.available() ) {     // Is there anything ready for us?
    
    RF24NetworkHeader header;        // If so, grab it and print it out
    memset(message, '\0', 300);
    network.read(header,&message,299);

    OSCBundle bndl;
    get_OSC_bundle(message, &bndl);
  

//// decode message and get data from sensors as indicated by the hub ////
// message indices: CO2 = 0, Lux = 1, Temp = 2, RH = 3, Dust = 4

  OSCMessage *msg = bndl.getOSCMessage(0);
  bool co2_use = msg->getInt(0);
  
  if(co2_use == true) {
    sendRequest(readCO2);  // get CO2 
  
    valCO2 = getValue(response); 

  }else{
    valCO2 = 0;
  }

  bool lux_use = msg->getInt(1);

  if(lux_use == true) {
    
    tsl.getEvent(&event); // get lux
  }else{
    event.light = 0;
  }

  bool temp_use = msg->getInt(2);

  if(temp_use == true) {
    
    t = sht31.readTemperature(); //get temperature
  }else{
    t = 0;
  }

  bool RH_use = msg->getInt(3);

  if(RH_use == true) {
    
    h = sht31.readHumidity(); //get humidity
  }else{
    h = 0;
  }

  bool dust_use = msg->getInt(4);

  if(dust_use == true) {
    
    dust = sht31.readHumidity(); //get dust 
  }else{
    dust = 0;
  }
  }
  
  DateTime now = rtc.now(); // get dateTime

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
    dataFile.print(valCO2);
    dataFile.print(',');
    dataFile.print(event.light);
    dataFile.print(',');
    dataFile.print(t);
    dataFile.print(',');
    dataFile.println(h);
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
  Serial.println(now.second(), DEC);
  Serial.print(valCO2);
  Serial.print(',');
  Serial.print(event.light);
  Serial.print(',');
  Serial.print(t);
  Serial.print(',');
  Serial.println(h);
  
  //delay (2000);// delay the logging time for x ms
  
  bool sent_flag = false;
  int attempts = 0; 
  do{

    OSCBundle bndl_send;
    bndl_send.add("randomaddress").add((int) valCO2).add((int)(event.light)).add((int)t).add((int)h);
     
    char message_send[300];
    memset(message_send, '\0', 300);
  
    get_OSC_string(&bndl_send,message_send);
  
    //print_bundle(&bndl_send);
  
    RF24NetworkHeader header_send(other_node);
    bool ok = network.write(header_send, message_send,strlen(message_send));
  
    if(ok == false){
      Serial.println("Total fail...resending.");
      attempts++;
      Serial.print("Attempts: ");
      Serial.println(attempts);
      delay(1000);
    }else{
      Serial.println("Sent!");
      sent_flag = true;
    }

  }while(sent_flag == false);   
  delay(1000);
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
    unsigned long val = high*256 + low;          //Combine high byte and low byte with this formula to get value
    return val* valMultiplier;
}

