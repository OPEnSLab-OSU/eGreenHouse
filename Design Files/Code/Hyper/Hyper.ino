/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// This is the HyperDrive Code.
// This program will get the coordinates from Hub_Tramsmit Code
// then move to that location with that speed and rotation.  
// then send only Location, and HyperRail Movement Status to eGreenhouse Sensor Package.
//
// Author: Kenneth Kang
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <ArduinoJson.h>                                                                                       // Need to include for the JsonDocument
#include <Loom.h>                                                                                              // Need to include the Loom Package into the program

#include "hyperJSON.h"                                                                                         // Include the JSON Package constructor
#include "HyperRail_Driver.h"

const char* json_config =                                                                                      // Include Configuration
#include "config.h"
;

int checker;                                                                                                   // Declare a global variable for checking th correct JSON package

LoomFactory<
  Enable::Internet::Disabled,                                                                                  // For GoogleSheet in Wifi/Ethernet,we need to enabled it
  Enable::Sensors::Disabled,                                                                                   // For measureing data 
  Enable::Radios::Enabled,                                                                                     // For Communcation between boards
  Enable::Actuators::Disabled,                                                                                 // For Enabling Motors & Relay (one-pin)
  Enable::Max::Disabled                                                                                        // Never ever Enable this part: It will kill the program
> ModuleFactory{};

LoomManager Loom{ &ModuleFactory };                                                                            // Having all of the properties of LoomFactory<> in a manager called Loom

void setup() {                                                                                                 // Put your setup code here, to run once:

  Serial.begin(9600);

  // axis Bump Switches 
  pinMode(N0Bump, INPUT_PULLUP); 
  pinMode(MaxBump, INPUT_PULLUP); 

  // Motor Control Pins
  pinMode(STEP, OUTPUT); 
  pinMode(DIR, OUTPUT); 

  // Set Max Speed For Stepper 
  stepper.setMaxSpeed(MaxSpeed/4);

  // Set the Acceleration for the stepper motors
  stepper.setAcceleration(80);
  
 //  initialize all interrupts for Bump Switches 
 attachInterrupt(digitalPinToInterrupt(N0Bump), ISR, FALLING);

 attachInterrupt(digitalPinToInterrupt(MaxBump), Max_ISR, FALLING);

  N0Flag = 0; 
  MAXFlag = 0;
  
  Loom.begin_serial(true);                                                                                     // Start the Serial over Loom
  Loom.parse_config(json_config);                                                                              // Add the config.h file into the program
  Loom.print_config();                                                                                         // Print out the config file if it works 

  LPrintln("\n ** Hyper Ready ** ");                                                                           // Indicating the user that setup function is complete
}

void loop() {                                                                                                  // Put your main code here, to run repeatedly:

  hyper_Base in_data;


   if(Loom.LoRa().receive_blocking_raw(in_data.raw, sizeof(in_data.raw), 10000)){                              // If LoRa receive something, then start these statments
    JsonObject internal_json = Loom.internal_json(true);                                                       // Create a new JSON that will be converted from the struct that came
    struct_to_json(in_data, internal_json);                                                                    // Add stuct data to JSON
    const JsonObject coordinates_json = Loom.internal_json(false);                                             // Open the JSON from the code
    const JsonArray contents = coordinates_json["contents"];                                                   // For simple syntax uses
    
    checker = contents[0]["data"]["B"];                                                                        // Update the checker value
    
    if (checker == -1){                                                                                        // Check if the board got the right JSON, if not, then it will move the else statement
      
    LPrintln("Got the user input Coordinate values");                                                          // Tell the user that we got the correct JSON
    
    Location = contents[1]["data"]["MM"];                                                                   // Get the Z_Location from JSON
    MaxSpeed = contents[2]["data"]["Velocity"];                                                                // Get the MaxSpeed from JSON
    Spool_Rad = contents[3]["data"]["Radius"];                                                            // Get the Spool_Rad_YZ from JSON
    Goto = contents[4]["data"]["B"];                                                                           // Get the GoTo from JSON
    looP = contents[5]["data"]["B"];                                                                           // Get the Loop from JSON
    Reset = contents[6]["data"]["B"];                                                                          // Get the Reset from JSON
    calibrate = contents[7]["data"]["B"];                                                                     // Get the Calibrate from JSON
    period = contents[8]["data"]["Num"];                                                                     // Get the Period from JSON
    
    int steps = mmToSteps(Location, SPR, Spool_Rad, Micro );
    if(Goto == 1){
      GoTo(steps); 
    }
    if(looP == 1){
      Loop(); 
    }

//    if(Reset == 1 and calibrate == 1){
//      GoTo(X0_pos, Y0_pos, Z0_pos);   
//    }
  // Serial.print(xAMove);

  // checkInts(); 


 // X0AFlag = checkInts(X0AFlag, X0ABump, X0A_pos, stepperX, xMove);

  // if rail is not calibrated then calibrate it 
//    if(calibrate == 1){
//      Calibrate(); 
//    }

    Loom.pause();
    contents[0]["data"]["B"] = 2;                                                                              // Update the checker value that it moved

    for (int i = 0; i < 7; i++){                          
      contents.remove(2);                                                                                      // Remove everything except x, y, z, and Hyper value from the JSON
    }
    
    Loom.display_data();                                                                                       // Display the new JSON to send the Sensor Package
    
    Loom.LoRa().send(9);                                                                                       // Send the JSON to the Sensor Package to measure values
    }
    else{
      LPrintln("Failed to get the Coordinates");                                                               // If the coordinate json was not sent, then it will print out this message
    } 
  }
}