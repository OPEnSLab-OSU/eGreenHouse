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
const uint16_t this_node = 00;
const uint16_t other_node = 01;
char message_received[300];

//Declare pin functions on MO
#define stp 9
#define dir 8
//#define MS1 4
//#define MS2 5
//#define MS3 6
#define EN  11
// Note: ethernet shield uses pin 10

//define ethernet//

#define DEVICE    "TestDevice"
#define INIT_INST 42
#define MAX_FIELDS 32			// Maximum number of fields accepted by the PushingBox Scenario

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
IPAddress ip(128,193,56,138); 						  // device's IP address

EthernetClient ethernet_client;

int           led = LED_BUILTIN;

void sendToPushingBox(OSCMessage &msg);
void sendToPushingBox(OSCBundle *bndl);
bool setup_ethernet();

void setup() {
  pinMode(stp, OUTPUT);
  pinMode(dir, OUTPUT);
  //pinMode(MS1, OUTPUT);
  //pinMode(MS2, OUTPUT);
  //pinMode(MS3, OUTPUT);
  pinMode(EN, OUTPUT);
  resetBEDPins(); //Set step, direction, microstep and enable pins to default states
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

  //delay(1500);//for testing
  //Serial.print("This is what is the value of Serial.available(): " );//for testing
  //Serial.println(Serial.available());//for testing

  //Wee need to wait for the serial port to start reading
  //because otherwise we don't see the "R"
  //This is beause the M0 is REALLY FAST!
  while(!Serial)
  {
    ;
  }

  //Tell Processing that we're ready
  Serial.println("R");

  while (Serial.available() == 0)
  {
    ;//Serial.println("Waiting");//for testing
  }

  //while(Serial.available() >= 2){
  //Serial.println("This is what is stored in \"user_input\" before the read : " );// for testing
  //Serial.println(user_input);//for testing
  //user_input = Serial.read(); //Read user input and trigger appropriate function
  //Serial.println("This is what is stored in \"user_input\" after the read : " );//for testing
  //Serial.println(user_input);//for testing

  decoder();//This is where the data gets parsed to get the values from the GUI



  digitalWrite(EN, LOW); //Pull enable pin low to set FETs active and allow motor control
  if (option == 1) {
    if(delay_time >= 5100){ //if this evaluates to true, then it will go slower than 1mm/s
      travelHyperRail_slow(total_steps, delay_time);
    }else{
      if(intervals_flag == 1){
        //Serial.println("Inside if");//for testing
        intervals_travelHyperRail(total_steps,delay_time,interval_steps,stops);
      }else{
      travelHyperRail(total_steps, delay_time);// Traveling faster than 1mm/s
        }
    }
  } else if (option == 2 || option == 5) {
    if(delay_time>=5100){//if this evaluates to true, then it will go slower than 1mm/s
      StepForwardDefault_slow(total_steps, delay_time);
    }else{
      StepForwardDefault(total_steps, delay_time);// Traveling faster than 1mm/s
    }
  }else if (option == 3 || option == 4) {
    if(delay_time >=5100){//if this evaluates to true, then it will go slower than 1mm/s
      StepBackwardDefault_slow(total_steps, delay_time);
    }else{
      StepBackwardDefault(total_steps, delay_time);// Traveling faster than 1mm/s
    }
    //user_input = Serial.read();//This will clear the buffer
  }else{
    Serial.println("Invalid input");
  }
  //Serial.print("This is what is the value of Serial.available() after if statments: " );//for testing
  //Serial.println(Serial.available());//for testing

  delay(1000);//Making sure it stops completeley before cutting current to motor.s
  resetBEDPins();
  //}
}

/******************* Function: decoder()*********************
   Description: This function will parse the message from processing
                into the corresponding varibles.These will be: option,
                steps, and delay time.
   Parameters: NONE
   Returns: VOID
*/
void decoder()
{

  if (Serial.available() > 0)
  {
    String data_str = Serial.readString();//reads in the string
    Serial.print("From uC side: ");// for testing
    Serial.println(data_str);//For testing
    Serial.print("String length: ");// for testing
    Serial.println(data_str.length());// for testing
    data_str.toCharArray(inData, 80);//converts the string into char array

  }
  //Serial.print("Here");// for testing
  //int numVals = sscanf(inData, "[O:%d,S:%ld,D:%d,I:%d,IS:%ld,ST:%d,CO2:%d,Lux:%d,Part:%d,HT:%d*/ ", &option, &total_steps, &delay_time, &intervals_flag, &interval_steps, &stops, &co2_activated, &lux_activated, &particle_activated, &humidity_activated);
  int numVals = sscanf(inData, "[O:%d,S:%ld,D:%d,I:%d,IS:%ld,ST:%d,CO2:%d,Lux:%d,Part:%d,H:%d,T:%d,TI:%d",
                                &option, &total_steps, &delay_time, &intervals_flag, &interval_steps,
                                &stops, &co2_activated, &lux_activated, &particle_activated, &humidity_activated,
                                &temperature_activated, &time_interval);
  /*Serial.print("Number of values parsed: ");//for testing
  Serial.println(numVals);// for testing
  //Serial.print("total_steps: ");//for testing
  //Serial.println(total_steps);//for testing
  //Serial.print("delay_time_A: ");// for testing
  //Serial.println(delay_time);// for testing
  Serial.print("intervals_flag: ");// for testing
  Serial.println(intervals_flag); // for testing
  //Serial.print("interval_steps: ");// for testing
  //Serial.println(interval_steps);//for testing
  Serial.print("Stops: ");// for testing
  Serial.println(stops);//for testing
  Serial.print("CO2: ");// for testing
  Serial.println(co2_activated);// for testing
  Serial.print("Lux: ");// for testing
  Serial.println(lux_activated);// for testing
  Serial.print("Particle: " );// for testing
  Serial.println(particle_activated);//for testing
  Serial.print("Humidity: ");// for testing
  Serial.println(humidity_activated);// for testing
  Serial.print("Temperature: " );// for testing
  Serial.println(temperature_activated);// for testing
  Serial.print("Time Interval Received: " );// for testing
  Serial.println(time_interval);// for testing*/
}



/******************* Function: resetBEDPins()*********************
   Description: This function resets the varialbe used for the states back to
                their original values
   Parameters: NONE
   Returns: VOID
*/
void resetBEDPins()
{
  digitalWrite(stp, LOW);
  digitalWrite(dir, LOW);
  //digitalWrite(MS1, LOW);
  //digitalWrite(MS2, LOW);
  //digitalWrite(MS3, LOW);
  digitalWrite(EN,HIGH); //If this is low, the motor will always be on holding its position.
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

void transmit_nRF()
{
network.update();                        // Check the network regularly
  Serial.println("network updated");//for testing
  bool sent_flag = false;
  int attempts = 0;

  do{

    Serial.println("Sending...");

    OSCBundle bndl;
    bndl.add("addr").add((int) co2_activated ).add((int) lux_activated).add((int) temperature_activated).add((int) humidity_activated).add( (int) particle_activated).add( (location/1000));

    char message[300];

    memset(message, '\0', 300);

    convert_OSC_bundle_to_string(&bndl, message);
    //char msg[120];
    //strcpy(msg, message);

    Serial.println("Testing");
    RF24NetworkHeader header(/*to node*/ other_node);
    Serial.println("testing");

    bool ok = network.write(header,message,strlen(message));

    delay(500);

    if(ok == false){
      Serial.println("Total fail...resending.");
      attempts++;
      Serial.print("Attempts: " );
      Serial.println(attempts);
    }else{
      Serial.println("Sent!");
      sent_flag = true;
    }
  }while(sent_flag == false);

   delay (3000);

   while(network.available() == 0 ){
    network.update(); //update the network while there is nothing available yet
    Serial.println("Waiting to receive something on transmitting end.");
   }

   while(network.available()){
    RF24NetworkHeader header2;
    memset(message_received, '\0', 300);
    network.read(header2, &message_received, 299);
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


/******************************Function: travelHyperRail()**********************************
   Description:This funciton will move the carriage the whole path length forwards and back.
   Parameters:
            -steps_total: This will be passed from the stepsper_length function.
                          This number dictates the total number of steps needed to
                          travel the whole rail.
   Returns: N/A. It moves the carriage up and down the rail.
*/
void travelHyperRail(long steps_total, int delay_time)
{
  //Serial.println("Traveling the HyperRail!");// for testing

  //Serial.print("My delay time is: " );//for testing
  //Serial.println(delay_time);//For testing
  //Serial.print("My total steps are: ");//for testing
  //Serial.println(steps_total);//for testing

  digitalWrite(dir, LOW); //Pull direction pin low to move "forward"
  unsigned long startTime = micros();// start time // for testing
  for (x = 1; x <= steps_total; x++) //Loop the forward stepping enough times for motion to be visible
  {
    digitalWrite(stp, HIGH); //Trigger one step forward
    delayMicroseconds(delay_time);
    digitalWrite(stp, LOW); //Pull step pin low so it can be triggered again
    delayMicroseconds(delay_time);

    //Serial.println(x);
  }

//// update location of sensors along the rail ////
    current_step = steps_total;
    location = sensors_position(current_step);


//// send data before returning ////
transmit_nRF();
/*
  unsigned long endTime = micros();//end time//for testing

  unsigned long totalTime = endTime - startTime;//for testing
  Serial.print("Microseconds: ");//for testing
  Serial.println(totalTime);//for testing
  Serial.print("Seconds: " );//for testing
  Serial.println(float(totalTime / (1 * pow(10,6)))); //for testing
*/


  //Waits 100 milliseconds before going back the other way
  delay(500);
  //Serial.println("I made it after the delay. Will now be going back");// for testing

  //This for loop will bring the carriage back to
  // the orignal postion
  digitalWrite(dir, HIGH);//Pull direction pin to HIGH to move "Backward"
  x = 0;
  for (x = 1; x <= steps_total; x++)
  {
    digitalWrite(stp, HIGH);
    delayMicroseconds(delay_time);
    digitalWrite(stp, LOW);
    delayMicroseconds(delay_time);

    //Serial.println(x);//for testing


  }
  current_step = 0;
  location = sensors_position(current_step);
  transmit_nRF();

  //Serial.println("Enter new option!");
}



/******************************Function: travelHyperRail_slow()**********************************
   Description:This funciton will move the carriage the whole path length forwards and back. The
               main difference between this function and the other one is that this one will be used
               for speeds less than 1mm/s.
   Parameters:
            -steps_total: This will be passed from the stepsper_length function.
                          This number dictates the total number of steps needed to
                          travel the whole rail.
            - delay_time: This will tell the microcontroller how long to pause in between steps.
   Returns: N/A. It moves the carriage up and down the rail.
*/
void travelHyperRail_slow(long steps_total, int delay_time)
{
  //Serial.println("Using slow version");//for testing
  int delay_time_milli = delay_time/1000;
  //Serial.println("Traveling the HyperRail!");// for testing

  //Serial.print("My delay time is: " );//for testing
  //Serial.println(delay_time);//For testing
  //Serial.print("My total steps are: ");//for testing
  //Serial.println(steps_total);//for testing

  digitalWrite(dir, LOW); //Pull direction pin low to move "forward"
  unsigned long startTime = micros();// start time // for testing
  for (x = 1; x <= steps_total; x++) //Loop the forward stepping enough times for motion to be visible
  {
    digitalWrite(stp, HIGH); //Trigger one step forward
    delay(delay_time_milli);
    digitalWrite(stp, LOW); //Pull step pin low so it can be triggered again
    delay(delay_time_milli);
    //Serial.println(x);
  }

  unsigned long endTime = micros();//end time//for testing

  unsigned long totalTime = endTime - startTime;//for testing
  Serial.print("Microseconds: ");//for testing
  Serial.println(totalTime);//for testing
  Serial.print("Seconds: " );//for testing
  Serial.println(float(totalTime / (1 * pow(10,6)))); //for testing



  //Waits 100 milliseconds before going back the other way
  delay(500);
  //Serial.println("I made it after the delay. Will now be going back");// for testing

  //This for loop will bring the carriage back to
  // the orignal postion
  digitalWrite(dir, HIGH);//Pull direction pin to HIGH to move "Backward"
  x = 0;
  for (x = 1; x <= steps_total; x++)
  {
    digitalWrite(stp, HIGH);
    delay(delay_time_milli);
    digitalWrite(stp, LOW);
    delay(delay_time_milli);
    //Serial.println(x);//for testing
  }
  //Serial.println("Enter new option!");
}



/***************************** Function: StepForwardDefault()***********************
   Description: This function will move the carriage all the way down and stay
                at the end of the line. NO return.
   Parameters: -RPM: The speed at which the motor will be turning
   Return: N/A. This function will only move the carriage forward, returns nothing.

*/
void StepForwardDefault(long steps_total, int delay_time)
{
  //Serial.println("\nMoving carriage forward.");// for testing

  //Serial.print("My delay time is: " );//for testing
  //Serial.println(delay_time);//For testing
  //Serial.print("My total steps are: ");//for testing
  //Serial.println(steps_total);//for testing

  digitalWrite(dir, LOW); //Pull direction pin low to move "forward"
  for (x = 1; x < steps_total; x++) //Loop the forward stepping enough times for motion to be visible
  {
    digitalWrite(stp, HIGH); //Trigger one step forward
    delayMicroseconds(delay_time);
    digitalWrite(stp, LOW); //Pull step pin low so it can be triggered again
    delayMicroseconds(delay_time);
    //Serial.println(x);
  }
  //Serial.println("Enter new option");
}



/***************************** Function: StepForwardDefault_slow()***********************
   Description: This function will move the carriage all the way down and stay
                at the end of the line. NO return. This function is for when the carriage is
                moving slower than 1mm/s.
   Parameters: -RPM: The speed at which the motor will be turning
   Return: N/A. This function will only move the carriage forward, returns nothing.

*/
void StepForwardDefault_slow(long steps_total, int delay_time)
{
  int delay_time_milli = delay_time/1000;
  //Serial.println("\nMoving carriage forward.");// for testing

  //Serial.print("My delay time is: " );//for testing
  //Serial.println(delay_time);//For testing
  //Serial.print("My total steps are: ");//for testing
  //Serial.println(steps_total);//for testing

  digitalWrite(dir, LOW); //Pull direction pin low to move "forward"
  for (x = 1; x < steps_total; x++) //Loop the forward stepping enough times for motion to be visible
  {
    digitalWrite(stp, HIGH); //Trigger one step forward
    delay(delay_time_milli);
    digitalWrite(stp, LOW); //Pull step pin low so it can be triggered again
    delay(delay_time_milli);
    //Serial.println(x);
  }
  //Serial.println("Enter new option");
}

/***************************** Function: StepBackwardDefault()***********************
   Description: This function will move the carriage all the way down and stay
                at the end of the line. NO return.
   Parameters: steps, delay time
   Return: N/A. This function will only move the carriage forward, returns nothing.

*/
void StepBackwardDefault(long steps_total, int delay_time)
{
  int delay_time_milli = delay_time/1000;
  //Serial.println("\nMoving carriage backward.");// for testing
  //Serial.print("My delay time is: " );//for testing
  //Serial.println(delay_time);//For testing
  //Serial.print("My total steps are: ");//for testing
  //Serial.println(steps_total);//for testing
  digitalWrite(dir, HIGH); //Pull direction pin high to move "backward"
  for (x = 1; x < steps_total; x++) //Loop the forward stepping enough times for motion to be visible
  {
    digitalWrite(stp, HIGH); //Trigger one step forward
    delayMicroseconds(delay_time);
    digitalWrite(stp, LOW); //Pull step pin low so it can be triggered again
    delayMicroseconds(delay_time);
    //Serial.println(x);
  }
  //Serial.println("Enter new option");
}


/***************************** Function: StepBackwardDefault_slow()***********************
   Description: This function will move the carriage all the way down and stay
                at the end of the line. NO return. This function will be used when the
                carriage is moving at speeds lower than 1mm/s.
   Parameters: steps, delay time
   Return: N/A. This function will only move the carriage forward, returns nothing.

*/
void StepBackwardDefault_slow(long steps_total, int delay_time)
{
  int delay_time_milli = delay_time/1000;
  //Serial.println("\nMoving carriage backward.");// for testing
  //Serial.print("My delay time is: " );//for testing
  //Serial.println(delay_time);//For testing
  //Serial.print("My total steps are: ");//for testing
  //Serial.println(steps_total);//for testing
  digitalWrite(dir, HIGH); //Pull direction pin high to move "backward"
  for (x = 1; x < steps_total; x++) //Loop the forward stepping enough times for motion to be visible
  {
    digitalWrite(stp, HIGH); //Trigger one step forward
    delay(delay_time_milli);
    digitalWrite(stp, LOW); //Pull step pin low so it can be triggered again
    delay(delay_time_milli);
    //Serial.println(x);
  }
  //Serial.println("Enter new option");
}

/******************************Function: sensors_position****************************************
  Description: This function calculates the current position of the sensor package along the rail
  Parameters: current_step
  Returns: int (because we want to round to nearest millimeter)
*/

int sensors_position(int current_step){
  return ceil(((current_step*2*3.1415926535*spool_radius)/steps_per_revolution));
}


/******************************Function: intervals_travelHyperRail()**********************************
   Description:This funciton will move the carriage the whole path length forwards by interval and then
               go back the origin of the rail.
   Parameters:
            -steps_total: This will be passed from the stepsper_length function.
                          This number dictates the total number of steps needed to
                          travel the whole rail.
            -delay_time: this is time delay for each steps
   Returns: N/A. It moves the carriage up and down the rail.
*/
void intervals_travelHyperRail(long steps_total, int delay_time, long interval_steps, int stops )
{
  for(;;){

/// Take a reading at starting position ///
//transmit_nRF();

/// move HyperRail forward ///
  current_step = 0;
  for(int s = 1; s <= stops; s++){
    digitalWrite(dir, LOW); //Pull direction pin low to move "forward"
    current_step += interval_steps;
    location = sensors_position(current_step);

    for (x = 1; x <= interval_steps; x++) //Loop the forward stepping enough times for motion to be visible
    {
      digitalWrite(stp, HIGH); //Trigger one step forward
      delayMicroseconds(delay_time);
      digitalWrite(stp, LOW); //Pull step pin low so it can be triggered again
      delayMicroseconds(delay_time);
      //Serial.println(x);// for testing
    }
    Serial.println("Finished Interval");//

transmit_nRF();

  }

  //This for loop will bring the carriage back to
  // the orignal postion
  digitalWrite(dir, HIGH);//Pull direction pin to HIGH to move "Backward"
  x = 0;
  for (x = 1; x <= steps_total; x++)
  {
    digitalWrite(stp, HIGH);
    delayMicroseconds(delay_time);
    digitalWrite(stp, LOW);
    delayMicroseconds(delay_time);
    //Serial.println(x);//for testing
  }

current_step = 0;
location = sensors_position(current_step);
delay(time_interval*1000);

Serial.println("interval test");
Serial.println(time_interval*1000);
  }
}
