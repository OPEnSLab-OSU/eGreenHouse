/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// This is the Hub for eGreenhouse.
// This program will get take user input from Processing GUI that we made
// then send those values to the HyperDrive to move them.
// It also receive package from Sensor Package to publish to GoogleSheets.
// Both communcations use LoRa.

// If you want to checkout the GoogleSheets, find the link below
// https://docs.google.com/spreadsheets/d/1M0TS0sN5z6AMDMXzh-0gRyxqf6c6dWQTrbkpmNTeRXI/edit#gid=0

// Author: Kenneth Kang

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Loom.h>                                                                   // Need to include the Loom Package into the program
#include <ArduinoJson.h>                                                            // Need to include for the JsonDocument


#include "eGreenhouseJSON.h"                                                        // Include the JSON Package constructor

const char* json_config =                                                           // Include Configuration
#include "config.h"
;

LoomFactory<
  Enable::Internet::All,                                                            //For GoogleSheet in Wifi/Ethernet,we need to enabled it
  Enable::Sensors::Disabled,                                                        //For getting sensor data: We don't need it for this program
  Enable::Radios::Enabled,                                                          //For Communcation between boards
  Enable::Actuators::Disabled,                                                      //For Motors (It will be part in the Hyperdrive)
  Enable::Max::Disabled                                                             //Never ever Enable this part: It will kill the program
> ModuleFactory{};

LoomManager Loom{ &ModuleFactory };                                                 //Having all of the properties of LoomFactory<> in a manager called Loom

void setup() {                                                                      //Put your setup code here, to run once:
  Loom.begin_serial(true);                                                          //Starting Loom
  Loom.parse_config(json_config);                                                   //Getting Information in Config.h file
  Loom.print_config();                                                              //Printing out the config.h information to make sure it is running correctly or not

  LPrintln("\n ** Hub Ready ** ");                                                  //Indicating the user that setup function is complete

}

void loop() {                                                                       //Put your main code here, to run repeatedly:

  if(Serial.available()){                                                           // Read Json Object from Processing
    DynamicJsonDocument doc(400);                                                   // Create an DynamicJsonDocument to edit the JSON
    String JsonStr = Serial.readString();                                           // Get JSON.string() from user input from the Processing

    DeserializationError err = deserializeJson(doc, JsonStr);                       // Deserialize the JsonString 

    if (err){                                                                       // If there an error, then it will enter this condition
      Serial.print(F("deserializeJson() failed with code "));
      Serial.println(err.c_str());

      return ;
    }

    serializeJsonPretty(doc, JsonStr);                                              // Serialize JSON using the DynamicJsonDocument and User Input

    

    int X_Location = doc["X_Location"];                                             // Get the X_Location from JSON
    int Y_Location = doc["Y_Location"];                                             // Get the Y_Location from JSON
    int Z_Location = doc["Z_Location"];                                             // Get the Z_Location from JSON
    int MaxSpeed = doc["Velocity"];                                                 // Get the MaxSpeed from JSON
    int Spool_Rad_X = doc["SpoolRadX"];                                             // Get the Spool_Rad_X from JSON
    int Spool_Rad_YZ = doc["SpoolRadYZ"];                                           // Get the Spool_Rad_YZ from JSON

    doc.clear();                                                                    // Clear the JSON because we don't need it anymore
    
    Loom.add_data("X_Locatiton", "MM", X_Location);                                 // Add X_Location to be record and send to the other board
    Loom.add_data("Y_Locatiton", "MM", Y_Location);                                 // Add Y_Location to be record and send to the other board
    Loom.add_data("Z_Locatiton", "MM", Z_Location);                                 // Add Z_Location to be record and send to the other board
    Loom.add_data("MaxSpeed", "Velocity", MaxSpeed);                                // Add MaxSpeed to be record and send to the other board
    Loom.add_data("Spool_Rad_X", "Radius", Spool_Rad_X);                            // Add Spool_Rad_X to be record and send to the other board
    Loom.add_data("Spool_Rad_YZ", "Radius", Spool_Rad_YZ);                          // Add Spool_Rad_YZ to be record and send to the other board

    Loom.display_data();                                                            // Display printed new JSON formatted data on serial monitor to double check 
    Loom.LoRa().send(6);                                                            // Send out JSON to the HyperRail Code

    Loom.pause(5000);                                                              // Wait to finish both HyperRail and Sensor Package

    eGreenhouse_Base in_data;                                                       // Create a new struct to convert back JSON
    if(Loom.LoRa().receive_blocking_raw(in_data.raw, sizeof(in_data.raw), 5000)){   // Wait the package from the Sensor Package for 5 seconds. If not then it will not be publish
      JsonObject internal_json = Loom.internal_json(true);                          // Create a new JSON
      struct_to_json(in_data, internal_json);                                       // Convert incoming struct to JSON
      Loom.display_data();                                                          // Display printed new JSON formatted data on serial monitor to double check 
      Loom.GoogleSheets().publish();                                                // Publish to GoogleSheets
    }
    
  }
}
