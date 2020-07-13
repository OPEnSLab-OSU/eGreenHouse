/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// This is the HyperDrive Code.
// This program will get the coordinates from Hub_Tramsmit Code
// then move to that location with that speed and rotation.  
// then send only X, Y, Z Location, and HyperRail Movement Status to eGreenhouse Sensor Package.
//
// Author: Kenneth Kang & Liam Duncan
//
// While Kenneth Kang wrote the code for communication, Liam Duncan wrote the HyperRail Movement code. 
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <ArduinoJson.h>                                                // Need to include for the JsonDocument
#include <Loom.h>                                                       // Need to include the Loom Package into the program

#include "hyperJSON.h"

const char* json_config =                                               // Include Configuration
#include "config.h"
;



int checker;                                                            // Declare a global variable for checking th correct JSON package

LoomFactory<
  Enable::Internet::Disabled,                                           // For GoogleSheet in Wifi/Ethernet,we need to enabled it
  Enable::Sensors::Disabled,                                            // For measureing data 
  Enable::Radios::Enabled,                                              // For Communcation between boards
  Enable::Actuators::Disabled,                                          // For Enabling Motors & Relay (one-pin)
  Enable::Max::Disabled                                                 // Never ever Enable this part: It will kill the program
> ModuleFactory{};

LoomManager Loom{ &ModuleFactory };                                     // Having all of the properties of LoomFactory<> in a manager called Loom

void setup() {                                                          // Put your setup code here, to run once:

  Loom.begin_serial(true);                                              // Start the Serial over Loom
  Loom.parse_config(json_config);                                       // Add the config.h file into the program
  Loom.print_config();                                                  // Print out the config file if it works 

  LPrintln("\n ** Hyper Ready ** ");                                    // Indicating the user that setup function is complete
}

void loop() {                                                           // Put your main code here, to run repeatedly:

  hyper_Base in_data;


   if(Loom.LoRa().receive_blocking_raw(in_data.raw, sizeof(in_data.raw), 10000)){                              // If LoRa receive something, then start these statments
    JsonObject internal_json = Loom.internal_json(true);
    struct_to_json(in_data, internal_json);    
    const JsonObject coordinates_json = Loom.internal_json(false);      // Open the JSON from the code
    const JsonArray contents = coordinates_json["contents"];            // For simple syntax uses
    
    checker = contents[0]["data"]["B"];                              // Update the checker value
    
    if (checker == -1){                                                 // Check if the board got the right JSON, if not, then it will move the else statement
      
    LPrintln("Got the user input Coordinate values");                   // Tell the user that we got the correct JSON

    Loom.display_data();
    
    int X_Location = contents[1]["data"]["MM"];                         // Get the X_Location from JSON
    int Y_Location = contents[2]["data"]["MM"];                         // Get the Y_Location from JSON
    int Z_Location = contents[3]["data"]["MM"];                         // Get the Z_Location from JSON
    int MaxSpeed = contents[4]["data"]["Velocity"];                     // Get the MaxSpeed from JSON
    int Spool_Rad_X = contents[5]["data"]["Radius"];                    // Get the Spool_Rad_X from JSON
    int Spool_Rad_YZ = contents[6]["data"]["Radius"];                   // Get the Spool_Rad_YZ from JSON
    int GoTo = contents[7]["data"]["B"];
    int Loop = contents[8]["data"]["B"];
    int Reset = contents[9]["data"]["B"];
    int Calibrate = contents[10]["data"]["B"];
    int PeriodX = contents[11]["data"]["Num"];
    int PeriodY = contents[12]["data"]["Num"];
    int PeriodZ = contents[13]["data"]["Num"];
    
    Loom.pause(10000);                                                  // This is where the function that the hyperRail moves(Everything related to this function is by Liam)
    
    contents[0]["data"]["B"] = 2;                                    // Update the checker value that it moved

    for (int i = 0; i < 10; i++){                          
      contents.remove(4);                                               // Remove MaxSpeed, Spool_Rad_x, and Spool_Rad_YZ value from the JSON
    }
    
    Loom.display_data();                                                // Display the new JSON to send the Sensor Package
    
    Loom.LoRa().send(9);                                                // Send the JSON to the Sensor Package to measure values
    }
    else{
      LPrintln("Failed to get the Coordinates");                        // If the coordinate json was not sent, then it will print out this message
    } 
  }
}
