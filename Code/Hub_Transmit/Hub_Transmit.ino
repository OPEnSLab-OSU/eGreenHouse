/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// This is the Hub_Transmit.
// This program will get take user input from Processing GUI that we made
// then send those values to the HyperDrive to move them.
//
// As you see that you also need to run .pde file to inport user input.
// Please take a look at that file also.
//
// Author: Kenneth Kang
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <Loom.h>                                                                 // Need to include the Loom Package into the program
#include <ArduinoJson.h>                                                          // Need to include for the JsonDocument

const char* json_config =                                                         // Include Configuration
#include "config.h"
;

int Location;                                                                     // Initialize the Location
int MaxSpeed;                                                                     // Initialize the MaxSpeed
int Spool_Rad;                                                                    // Initialize the Spool_Rad
int GoTo;                                                                         // Initialize the GoTo
int Loop;                                                                         // Initialize the Loop
int Reset;                                                                        // Initialize the Reset
int Calibrate;                                                                    // Initialize the Calibrate
int Period;                                                                       // Initialize the Period

LoomFactory<
  Enable::Internet::Disabled,                                                     // For GoogleSheet in Wifi/Ethernet,we need to disabled it
  Enable::Sensors::Disabled,                                                      // For getting sensor data: We don't need it for this program
  Enable::Radios::Enabled,                                                        // For Communcation between boards
  Enable::Actuators::Disabled,                                                    // For Motors
  Enable::Max::Disabled                                                           // Never ever Enable this part: It will kill the program
> ModuleFactory{};

LoomManager Loom{ &ModuleFactory };                                               // Having all of the properties of LoomFactory<> in a manager called Loom

void setup() {                                                                    // Put your setup code here, to run once:
  Loom.begin_serial(true);                                                        // Starting Loom
  Loom.parse_config(json_config);                                                 // Getting Information in Config.h file
  Loom.print_config();                                                            // Printing out the config.h information to make sure it is running correctly or not

  LPrintln("\n ** Hub_Transmit Ready ** ");                                       // Indicating the user that setup function is complete

}

void loop() {                                                                     // Put your main code here, to run repeatedly:

  if(Serial.available()){                                                         // Read Json Object from Processing                                                
    catchValue();                                                                 // Catch values from the User Input: Check line 69
    setValues();                                                                  // Add values to the JSON that will be send to the other board: check line 99                                                       
    updateValues();                                                               // Update the values in the JSON: check line 116
    Loom.display_data();                                                          // Display printed new JSON formatted data on serial monitor to double check 
    Loom.LoRa().send(6);
                                                             

  }
}

void catchValue(){                                                                // Catch values from the User Input from the GUI
    DynamicJsonDocument doc(400);                                                 // Create an DynamicJsonDocument to edit the JSON
    String JsonStr = Serial.readString();                                         // Get JSON.string() from user input from the Processing

    DeserializationError err = deserializeJson(doc, JsonStr);                     // Deserialize the JsonString 

    if (err){                                                                     // If there an error, then it will enter this condition
      Serial.print(F("deserializeJson() failed with code "));
      Serial.println(err.c_str());
      return ;
    }

    serializeJsonPretty(doc, JsonStr);                                            // Serialize JSON using the DynamicJsonDocument and User Input
    
    LPrintln("User Input Accepted");                                              // Telling the user that user input has accepted
    Location = doc["Location"];                                                   // Get the Location from JSON
    MaxSpeed = doc["Velocity"];                                                   // Get the MaxSpeed from JSON
    Spool_Rad = doc["SpoolRad"];                                                  // Geut the Spool_Rad from JSON
    GoTo = doc["GoTo"];                                                           // Get the GoTo from JSON
    Loop = doc["Loop"];                                                           // Get the Loop from JSON
    Reset = doc["Reset"];                                                         // Get the Reset from JSON
    Calibrate = doc["Calibrate"];                                                 // Get the Calibrate from JSON
    Period = doc["period"];                                                       // Get the Period from JSON
}

void setValues(){                                                                 // Update the certain values and added to the JSON Pacakge
    Loom.package();
    Loom.add_data("Location", "MM", 0);                                           // Add Location to the JSON to be record and send to the other board
    Loom.add_data("MaxSpeed", "Velocity", 0);                                     // Add MaxSpeed to the JSON to be record and send to the other board
    Loom.add_data("Spool_Rad", "Radius", 0);                                      // Add Spool_Rad to the JSON to be record and send to the other board
    Loom.add_data("GoTo", "B", 0);                                                // Add GoTo Boolean to the JSON to  be record and send to the other board
    Loom.add_data("Loop", "B", 0);                                                // Add Loop Boolean to the JSON to be record and send to the other board
    Loom.add_data("Reset", "B", 0);                                               // Add Reset Boolean to the JSON to be record and send to the other board
    Loom.add_data("Calibrate", "B", 0);                                           // Add Calibrate Boolean to the JSON to be record and send to the other board
    Loom.add_data("Period", "Num", 0);                                            // Add Period to the JSON to be record and send to the other board

}

void updateValues(){                                                              // Update the values after the setValues function 
    const JsonObject coordinates_json = Loom.internal_json(false);                // Open the JSON from the code
    const JsonArray contents = coordinates_json["contents"];                      // For simple syntax uses
    contents[1]["data"]["MM"] = Location;                                         // Update the Location value
    contents[2]["data"]["Velocity"] = MaxSpeed;                                   // Update the MaxSpeed value
    contents[3]["data"]["Radius"] = Spool_Rad;                                    // Update the Spool_Rad value
    contents[4]["data"]["B"] = GoTo;                                              // Update the GoTo value
    contents[5]["data"]["B"] = Loop;                                              // Update the Loop value
    contents[6]["data"]["B"] = Reset;                                             // Update the Reset value
    contents[7]["data"]["B"] = Calibrate;                                         // Update the Calibrate value
    contents[8]["data"]["Num"] = Period;                                          // Update the Period value
}
