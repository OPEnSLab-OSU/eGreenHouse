/*
  Openly Published Environmental Sensing Lab
  Oregon State University
  Written by: Jose Manuel Lopez Alcala + Lars Larson
  This code was based off Sparkfun's Big Easy Driver Basic Demo code
  https://github.com/sparkfun/Big_Easy_Driver

  This is the version that goes with Processing HyperSenseGUI.pde

  ***Use this version when the HyperRail hub has direct access to ethernet***

  This system's default will be 1/16 microstepping or 6180 steps per revolution
  using  a 0.9degree/step motor

This is the Feather M0 + Ethernet FeatherWing

*/


//Import libraries
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include <OSCBundle.h>
#include <Ethernet2.h>
#define LOOM_DEBUG 1 // enables serial printing (ethernet)
#include "loom_translator.h" // scary

//Define nRF stuff
RF24 radio(5,6);
RF24Network network(radio);
const uint16_t this_node = 02;
const uint16_t other_node = 00;

char message_received[300];


// Note: ethernet shield uses pin 10

//define ethernet//

#define DEVICE    "TestDevice"
#define INIT_INST 42
#define MAX_FIELDS 32      // Maximum number of fields accepted by the PushingBox Scenario

// Pushingbox / Spreadsheet stuff
#define spreadsheet_id "1IoahSla85lSKEajTm5zPOfVBMPgtBgZi4tSO6pHUrw8"
// Google Spreadsheet Sheet/Tab number. Sent as parameter to PushingBox/Google Scripts
#define tab_id  "Testing"
// Required by PushingBox, specific to each scenario
char device_id[]   = "vD24B53BD7320364"; // Lars' PushingBox scenario that sends to Lars' google script
// these are normally defined in the config.h file,
// but I put it here so you dont need all of the config file
#define is_lora 0

//Declare variable for functions
char user_input;
String inString = " "; // string to hold input
float path_length = 0; // length of rail in meters
float spool_radius = 10;// Radius of the spool that is holding the line
long total_steps = 0; //Total steps calculated to move all the way down the length of the rail.
long x = 0;//counting variable8
bool entered_numberLength = false;//Flag variable to check whether or not the input was a number and not a word
bool entered_numberRadius = false;//Flag variable to check whether or not hte input was a nubmer and not a word
bool entered_RPM = false;//Flag variable to check whether or not hte input was a nubmer and not a word
float RPM_HyperRail = 0;//This variable will be used to store the rpm of the systme after it is converted in the rpmtodelay function
int delay_time = 0;
int inPos = 0;
int intervals_flag = false;//Option for intervals
long interval_steps = 0;//# of steps per interval
int time_interval = 0; // # of seconds to wait before each interval
int stops = 0;//# of intervals
//ints to activate sensors
int co2_activated  = 0;
int lux_activated = 0;
int particle_activated = 0;
int humidity_activated = 0;
int temperature_activated = 0;
int steps_per_revolution = 6180;
float location = 0;
int option = 0; //this will be used to store value from processing

char inData[80];//This will be used for the incoming data from processing
int current_step = 0;


//Use this for OPEnS Lab ethernet
//byte mac[] = {0x00, 0x23, 0x12, 0x12, 0xCE, 0x7D};    // Luke's Mac's Mac Address
 byte mac[] = {0x98, 0x76, 0xB6, 0x10, 0x61, 0xD6};  //OPEnS Mac Address
IPAddress ip(128,193,56,138);               // device's IP address

EthernetClient ethernet_client;

int           led = LED_BUILTIN;

void sendToPushingBox(OSCMessage &msg);
void sendToPushingBox(OSCBundle *bndl);
bool setup_ethernet();

void setup() {

  
  //Serial.begin(9600); //Open Serial connection for debugging
  //Serial.println("OPEnS Lab HyperRails");

  SPI.begin();
  radio.begin();
  network.begin(/*channel*/ 90, this_node);

  #if LOOM_DEBUG == 1
    Serial.begin(9600);
    while(!Serial);        // Ensure Serial is ready to go before anything happens in LOOM_DEBUG mode.
    delay(5000);
    Serial.println("Initialized Serial!");
  #endif

  setup_ethernet();

}

// ================================================================
// ===                        ETHERNET                          ===
// ================================================================
bool setup_ethernet()
{
  Serial.println("Setting up ethernet");

  bool is_setup;
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }

  if (ethernet_client.connect("www.google.com", 80)) {
    is_setup = true;
    Serial.println("Successfully connected to internet");
    ethernet_client.stop();
  } else {
    is_setup = false;
    Serial.println("Failed to connect to internet");
  }

  return is_setup;
}

//Main loop
void loop() {

receive_nRF();

}


// --- SEND TO PUSHINGBOX ---
//
// Sends a get request to PushingBox
// Expects arguments of OSC message to be formated as:
// key, value, key, value...
// As the message router forwards messages not bundles, this is called by msg_router
// If you need to send a bundle, simply call the function (which is overloaded)
// with a correctly formatted OSCBundle
//
// The function tries platforms in the following order:
//   Ethernet, WiFi, cellular
// as enabled. The function returns immediately upon successful transmission
// else will try alternate platforms
// (this might become an option later, that is, whether or not to try other enabled platforms or not upon failure)
//
//
// @param msg  The message containing the information to send to PB.
//
void sendToPushingBox(OSCMessage &msg)
{
  if (msg.size() > 32) { // This also catches empty msgs, which seem to have a size around 1493 for some reason
    Serial.println("Message to large to send to PushingBox");
    return;
  }
  Serial.println("Sending to PushingBox");


  // Build url arguments from bundle
  char args[1024];
  sprintf(args, "/pushingbox?devid=%s&key0=sheetID&val0=%s&key1=tabID&val1=%s&key2=deviceID&val2=%s%d",
    device_id, spreadsheet_id, tab_id, DEVICE, INIT_INST);

  for (int i = 0, j = 3; (i < MAX_FIELDS-6) && (i < msg.size()); i+=2, j++) {
      char buf1[30], buf2[30];
    (get_data_value(&msg, i  )).toCharArray(buf1, 30);
    (get_data_value(&msg, i+1)).toCharArray(buf2, 30);
    sprintf(args, "%s&key%d=%s&val%d=%s", args, j, buf1, j, buf2);
  }
  //Serial.println("URL get args: ");
  //Serial.println(args);



  Serial.println("Running PushingBox for Ethernet");

  ethernet_client.stop();
  if (ethernet_client.connect("api.pushingbox.com", 80)) {
    Serial.println("Connection good");

    ethernet_client.print("GET ");
    ethernet_client.print(args);
    ethernet_client.println(" HTTP/1.1\nHost: api.pushingbox.com\nUser-Agent: Arduino\n");

    Serial.println("Data done sending");

    return;  // data sent successfully, no need to try another platform

  } else {
    Serial.println("No Connection");
    Serial.println("Failed to connect to PB, attempting to re-setup ethernet.");

    if (setup_ethernet()) {
      Serial.println("Successfully re-setup ethernet.");
    }
    #if LOOM_DEBUG == 1
    else {
      Serial.println("Failed to re-setup ethernet.");
    }
    #endif
  }

}


// --- SEND TO PUSHINGBOX ---
//
// Sends a get request to PushingBox
// Expects OSC bundle to only have 1 message and its arguments to be formated as:
// key, value, key, value...
// Simple a wrapper for the version of this function that takes a message
// As the message router forwards messages not bundles
//
// @param msg  The message containing the information to send to PB.
//
void sendToPushingBox(OSCBundle *bndl)
{
  OSCBundle tmpBndl;
  deep_copy_bundle(bndl, &tmpBndl);
  convert_bundle_structure(&tmpBndl, SINGLEMSG);
  #if LOOM_DEBUG == 1
    print_bundle(&tmpBndl);
  #endif
  sendToPushingBox(*(tmpBndl.getOSCMessage(0)));
}



/******************* Function: transmit_nRF()*********************
   Description: This function sends an OSC bundle to the sensor package over nRF
   Parameters: NONE
   Returns: VOID
*/

void receive_nRF()
{

   while(network.available() == 0 ){
    network.update(); //update the network while there is nothing available yet
    Serial.println("Waiting to receive something from middle node.");
    delay(1000);
   }

   while(network.available()){
    RF24NetworkHeader header_from_node;
    memset(message_received, '\0', 300);
    network.read(header_from_node, &message_received, 299);
    Serial.print("Message size: ");
    Serial.println(strlen(message_received));

    OSCBundle bndl_received;
    convert_OSC_string_to_bundle(message_received, &bndl_received);

    print_bundle(&bndl_received);


    OSCMessage *msg = bndl_received.getOSCMessage(0);
    int co2_val = msg->getInt(0);
    int lux_val = msg->getInt(1);
    int temp_val = msg->getInt(2);
    int RH_val = msg->getInt(3);
    int dust_val = msg->getInt(4);

    OSCBundle ethernet_bndl;
    String keys[5]   = {"CO2", "Lux", "Temp", "RH", "Particle"};
    float  values[5] = {co2_val, lux_val, temp_val, RH_val, dust_val};

    convert_assoc_arrays_to_bundle(keys, values,  &ethernet_bndl, "/an/address/or/something", 5, SINGLEMSG);
    sendToPushingBox(&ethernet_bndl);

    //while(1); // stop awhile
   }
}
