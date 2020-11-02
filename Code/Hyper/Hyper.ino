/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// This is the HyperDrive Code.
// This program will get the coordinates from Hub_Tramsmit Code
// then move to that location with that speed and rotation.  
// then send only Location, and HyperRail Movement Status to eGreenhouse Sensor Package.
//
// While Hyper.ino and config.h is for Radio communcation
// HyperRail_Driver.cpp and HyperRail_Driver.h is for the HyperRail movement code
//
// Author: Kenneth Kang
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <ArduinoJson.h>                                                                                       // Need to include for the JsonDocument
#include <Loom.h>                                                                                              // Need to include the Loom Package into the program

#include "HyperRail_Driver.h"

const char* json_config =                                                                                      // Include Configuration
#include "config.h"
;
                                                                                                   // Declare a global variable for checking th correct JSON package

LoomFactory<
  Enable::Internet::Disabled,                                                                                  // For GoogleSheet in Wifi/Ethernet,we need to enabled it
  Enable::Sensors::Disabled,                                                                                   // For measureing data 
  Enable::Radios::Enabled,                                                                                     // For Communcation between boards
  Enable::Actuators::Disabled,                                                                                 // For Enabling Motors & Relay (one-pin)
  Enable::Max::Disabled                                                                                        // Never ever Enable this part: It will kill the program
> ModuleFactory{};

LoomManager Loom{ &ModuleFactory };                                                                            // Having all of the properties of LoomFactory<> in a manager called Loom

void setup() {                                                                                                 // Put your setup code here, to run once:

  Serial.begin(9600);                                                                                          // Starting from here, it will be enabling the pins for the HyperRail

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

void loop(){
   if(Loom.LoRa().receive_blocking(10000)){                                                                    // If LoRa receive something, then start these statments
    Loom.pause(1000);                                                                                          // Wait for a second to get all the package that was sent
    const JsonObject coordinates_json = Loom.internal_json(false);                                             // Open the JSON from the code
    const JsonArray contents = coordinates_json["contents"];                                                   // For simple name for later usage
    const char* checker = coordinates_json["id"]["name"];                                                      // Update the checker value to make sure if board got the correct package
    if (strcmp(checker, "Hub") == 0){                                                                  // Check if the board got the right JSON, if not, then it will move the else statement
    LPrintln("[LoRa] Got the user input Coordinate values");                                                          // Tell the user that we got the correct JSON
    
    Location = contents[0]["data"]["MM"];                                                                      // Get the Location from JSON
    MaxSpeed = contents[1]["data"]["Velocity"];                                                                // Get the MaxSpeed from JSON
    Spool_Rad = contents[2]["data"]["Radius"];                                                                 // Get the Spool_Rad from JSON
    Goto = contents[3]["data"]["B"];                                                                           // Get the GoTo from JSON
    looP = contents[4]["data"]["B"];                                                                           // Get the Loop from JSON
    Reset = contents[5]["data"]["B"];                                                                          // Get the Reset from JSON
    calibrate = contents[6]["data"]["B"];                                                                      // Get the Calibrate from JSON
    period = contents[7]["data"]["Num"];                                                                       // Get the Period from JSON
    
    int steps = mmToSteps(Location, SPR, Spool_Rad, Micro );
    if(Goto == 1){
      GoTo(steps);                                                                                             // It will move the HyperRail
    }
    if(looP == 1){
      Loop(); 
    }

    Loom.pause(10000);                                                                                         // Pause the code for 10 seconds

    Loom.internal_json(true);                                                                                  // Reset the internal_json as empty
    Loom.package();                                                                                            // Add the Meta data into the internal_json
    Loom.add_data("Location", "MM", Location);                                                                 // Add Location variable to the internal_json
    
    Loom.display_data();                                                                                       // Display the new JSON to send the Sensor Package
    
    Loom.LoRa().send(9);                                                                                       // Send the JSON to the Sensor Package to measure values
    }
    else{
      LPrintln("Failed to get the Coordinates");                                                               // If the coordinate json was not sent, then it will print out this message
    } 
  }
}
