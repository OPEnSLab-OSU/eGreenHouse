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

LoomFactory<
  Enable::Internet::All,                                                            // For GoogleSheet in Ethernet, we need to enabled it
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

void loop() {  
  if(Loom.LoRa().receive_blocking(10000)){                                          // Wait for a package for 10 seconds, if not try again
      Loom.pause(1000);                                                             // Pause for getting all the data from the pacakge
      Loom.display_data();                                                          // Display the data what was send from eGH_Sensor_Pacakge
      const JsonObject complete_json = Loom.internal_json(false);                   // Set a new variable called complete_json
      const char* checker = complete_json["id"]["name"];                            // Set a new variable called checker to make sure if check the correct package
      if(strcmp(checker, "eGH_Package") == 0){                                      // If the package is correct, publish to GoogleSheets
        Loom.GoogleSheets().publish(complete_json);                                 // The link will be provided above
      }
  }
  Loom.pause(1000);                                                                 // Wait for a cooldown time for waiting mode for a second
}
