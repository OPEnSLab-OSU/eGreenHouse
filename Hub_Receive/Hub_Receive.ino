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

#include "eGreenhouseJSON.h"


const char* json_config =                                                           // Include Configuration
#include "config.h"
;

int checker;

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
    eGreenhouse_Base in_data;                                                       // Create a new struct to convert back JSON
    if(Loom.LoRa().receive_blocking_raw(in_data.raw, sizeof(in_data.raw), 15000)){   // Wait the package from the Sensor Package for 5 seconds. If not then it will not be publish
      JsonObject internal_json = Loom.internal_json(true);                          // Create a new JSON
      hub_struct_to_json(in_data, internal_json);                                       // Convert incoming struct to JSON
      Loom.display_data();                                                          // Display printed new JSON formatted data on serial monitor to double check 
      const JsonObject complete_json = Loom.internal_json(false);
      const JsonArray contents = complete_json["contents"];
      checker = contents[7]["data"]["B"];
      if(checker == 1){
        Loom.GoogleSheets().publish();
      }
      else{
        LPrintln("Failed to Publish to GoogleSheets");
      }
   }
}
