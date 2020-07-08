/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// This is the HyperDrive Code.
// This program will get the coordinates from Hub Code
// then move to that location with that speed and rotation.  
// then send only X, Y, and Z Location to eGreenhouse Sensor Package.

// Author: Kenneth Kang & Liam Duncan

// While Kenneth Kang wrote the code for communication, Liam Duncan wrote the HyperRail Movement code. 

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

 #include <ArduinoJson.h>                                               // Need to include for the JsonDocument
 #include <Loom.h>                                                      // Need to include the Loom Package into the program

 const char* json_config =                                              // Include Configuration
 #include "config.h"
 ;

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

// main Loop 
void loop() {

   if(Loom.LoRa().receive_blocking(5000)){                              // If LoRa receive something, then start these statments
    
    Loom.display_data();                                                // Display printed JSON formatted data on serial monitor

    DynamicJsonDocument doc(400);                                       // Create an DynamicJsonDocument to edit the JSON
    
    const JsonObject coordinates_json = Loom.internal_json(false);      // Create JsonObject that was received from the Hub

    const JsonArray contents = coordinates_json["contents"];            // Create a JsonArray from the JSON 
        
    int X_Location = contents[0]["data"]["MM"];                         // Get the X_Location from JSON
    int Y_Location = contents[1]["data"]["MM"];                         // Get the Y_Location from JSON
    int Z_Location = contents[2]["data"]["MM"];                         // Get the Z_Location from JSON
    int MaxSpeed = contents[3]["data"]["Velocity"];                     // Get the MaxSpeed from JSON
    int Spool_Rad_X = contents[4]["data"]["Radius"];                    // Get the Spool_Rad_X from JSON
    int Spool_Rad_YZ = contents[5]["data"]["Radius"];                   // Get the Spool_Rad_YZ from JSON

    doc.clear();                                                        // Clear the JSON because we don't need it anymore
   
    Loom.add_data("X_Locatiton", "MM", X_Location);                     // Add X_Location to be record and send to the other board
    Loom.add_data("Y_Locatiton", "MM", Y_Location);                     // Add Y_Location to be record and send to the other board
    Loom.add_data("Z_Locatiton", "MM", Z_Location);                     // Add Z_Location to be record and send to the other board
    
    Loom.display_data();                                                // Display printed new JSON formatted data on serial monitor to double check 
   
    Loom.LoRa().send(9);                                                // Send out JSON to the Sensor Package code 
    
   }

 }
