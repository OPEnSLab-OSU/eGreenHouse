/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// This is the Hub_Receive for eGreenhouse.
// It will receive package from Sensor Package to publish to GoogleSheets.
//
// If you want to checkout the GoogleSheets, find the link below
// https://docs.google.com/spreadsheets/d/1M0TS0sN5z6AMDMXzh-0gRyxqf6c6dWQTrbkpmNTeRXI/edit#gid=0
//
// Author: Kenneth Kang
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <Loom.h>                                                                   // Need to include the Loom Package into the program
#include <ArduinoJson.h>                                                            // Need to include for the JsonDocument

const char* json_config =                                                           // Include Configuration
#include "config.h"
;

LoomFactory<
  Enable::Internet::All,                                                            // For GoogleSheet in Wifi/Ethernet,we need to enabled it
  Enable::Sensors::Disabled,                                                        // For getting sensor data: We don't need it for this program
  Enable::Radios::Enabled,                                                          // For Communcation between boards
  Enable::Actuators::Disabled,                                                      // For Motors (It will be part in the Hyperdrive)
  Enable::Max::Disabled                                                             // Never ever Enable this part: It will kill the program
> ModuleFactory{};

LoomManager Loom{ &ModuleFactory };                                                 // Having all of the properties of LoomFactory<> in a manager called Loom

void setup() {                                                                      // Put your setup code here, to run once:
  Loom.begin_serial(true);                                                          // Starting Loom
  Loom.parse_config(json_config);                                                   // Getting Information in Config.h file
  Loom.print_config();                                                              // Printing out the config.h information to make sure it is running correctly or not

  LPrintln("\n ** Hub Ready ** ");                                                  // Indicating the user that setup function is complete

}

void loop() {                                                                       // Put your main code here, to run repeatedly:
    if(Loom.LoRa().receive_blocking(1000)){                                         // Wait the package from the Sensor Package for 1 seconds. If not then it will not be publish
      Loom.display_data();                                                          // Display printed new JSON formatted data on serial monitor to double check 
      const JsonObject complete_json = Loom.internal_json(false);                   // Open the JSON from code
      const JsonArray contents = complete_json["contents"];                         // For simple syntax use
      const char* checker = complete_json["id"]["name"];                             // Add a new variable called checker to make sure if the board got the correct package
      if(strcmp(checker, "eGH_Package")){                                           // Check if the board got the correct package
        Loom.GoogleSheets().publish();                                              // It will publish the data into GoogleSheets: check the the link in line 7
      }
      else{
        LPrintln("Failed to Publish to GoogleSheets");                              // Else, it will print this statement
      }
   }
}
