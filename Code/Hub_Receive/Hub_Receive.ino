/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// This is the Hub_Receive for eGreenhouse.
// It will receive package from Sensor Package to publish to GoogleSheets.
//
// If you want to checkout the GoogleSheets, find the link below
// https://docs.google.com/spreadsheets/d/1JpGKZocPMZNVpUIcMXa8LL-qB4djc1oBNfVQdpjY-qI/edit#gid=0
//
// Author: Kenneth Kang
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <Loom.h>                                                                   // Need to include the Loom Package into the program
#include <ArduinoJson.h>                                                            // Need to include for the JsonDocument

const char* json_config =                                                           // Include Configuration
#include "config.h"
;

int checker;                                                                        // Initialize checker

LoomFactory<
  Enable::Internet::Ethernet,                                                       // For GoogleSheet in Ethernet, we need to enabled it
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

  LPrintln("\n ** Hub_Recieve Ready ** ");                                          // Indicating the user that setup function is complete

}

void loop() {                                                                       // Put your main code here, to run repeatedly:
    if(Loom.LoRa().receive_blocking(5000)){                                        // Wait the package from the Sensor Package for 30 seconds. If not then it will not be publish
      Loom.display_data();                                                          // Display printed new JSON formatted data on serial monitor to double check 
      const JsonObject complete_json = Loom.internal_json(false);                   // Open the JSON from code
      const JsonArray contents = complete_json["contents"];                         // For simple syntax use
      checker = contents[6]["data"]["Bool"];                                        // Update the checker value
      if(checker == 1){                                                             // If the checker value is equal to 1(It tells that it came from the eGH_Sensor_Package)
                                                                                    // If local time is enable from the eGH_Sensor_Package, then set as 6, else 5 in line 45
        Loom.GoogleSheets().publish();                                              // It will publish the data into GoogleSheets: check the the link in line 7
      }
      else{
        LPrintln("Failed to Publish to GoogleSheets");                              // Else, it will print this statement
      }
   }
}
