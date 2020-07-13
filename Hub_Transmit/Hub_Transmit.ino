/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// This is the Hub_Transmit for eGreenhouse.
// This program will get take user input from Processing GUI that we made
// then send those values to the HyperDrive to move them.
//
// As you see that you also need to run .pde file to inport user input.
// Please take a look at that file also.
//
// If you want to checkout the GoogleSheets, find the link below
// https://docs.google.com/spreadsheets/d/1M0TS0sN5z6AMDMXzh-0gRyxqf6c6dWQTrbkpmNTeRXI/edit#gid=0
//
// Author: Kenneth Kang
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <Loom.h>                                                                 // Need to include the Loom Package into the program
#include <ArduinoJson.h>                                                          // Need to include for the JsonDocument

#include "hyperJSON.h"                                                            // Include the JSON Package constructor

const char* json_config =                                                         // Include Configuration
#include "config.h"
;

int X_Location;                                                                   // Initialize the X_Location
int Y_Location;                                                                   // Initialize the Y_Location
int Z_Location;                                                                   // Initialize the Z_Location
int MaxSpeed;                                                                     // Initialize the MaxSpeed
int Spool_Rad_X;                                                                  // Initialize the Spool_Rad_X
int Spool_Rad_YZ;                                                                 // Initialize the Spool_Rad_YZ
int GoTo;                                                                         // Initialize the GoTo
int Loop;                                                                         // Initialize the Loop
int Reset;                                                                        // Initialize the Reset
int Calibrate;                                                                    // Initialize the Calibrate
int PeriodX;                                                                      // Initialize the PeriodX
int PeriodY;                                                                      // Initialize the PeriodY
int PeriodZ;                                                                      // Initialize the PeriodZ

LoomFactory<
  Enable::Internet::Disabled,                                                     // For GoogleSheet in Wifi/Ethernet,we need to enabled it
  Enable::Sensors::Disabled,                                                      // For getting sensor data: We don't need it for this program
  Enable::Radios::Enabled,                                                        // For Communcation between boards
  Enable::Actuators::Disabled,                                                    // For Motors (It will be part in the Hyperdrive)
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
    hyper_Base out_struct;
    const JsonObjectConst internal_data = Loom.internal_json(false);
    json_to_struct(internal_data, out_struct);
    Loom.LoRa().send_raw(out_struct.raw, sizeof(out_struct.raw), 6);
                                                             

  }
}

void catchValue(){
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
    X_Location = doc["X_Location"];                                               // Get the X_Location from JSON
    Y_Location = doc["Y_Location"];                                               // Get the Y_Location from JSON
    Z_Location = doc["Z_Location"];                                               // Get the Z_Location from JSON
    MaxSpeed = doc["Velocity"];                                                   // Get the MaxSpeed from JSON
    Spool_Rad_X = doc["SpoolRadX"];                                               // Get the Spool_Rad_X from JSON
    Spool_Rad_YZ = doc["SpoolRadYZ"];                                             // Get the Spool_Rad_YZ from JSON
    GoTo = doc["GoTo"];                                                           // Get the GoTo from JSON
    Loop = doc["Loop"];                                                           // Get the Loop from JSON
    Reset = doc["Reset"];                                                         // Get the Reset from JSON
    Calibrate = doc["Calibrate"];                                                 // Get the Calibrate from JSON
    PeriodX = doc["periodX"];                                                     // Get the PeriodX from JSON
    PeriodY = doc["periodY"];                                                     // Get the PeriodY from JSON
    PeriodZ = doc["periodZ"];                                                     // Get the PeriodZ from JSON
}

void setValues(){
    Loom.add_data("Hyper", "B", -1);
    Loom.add_data("X_Location", "MM", 0);                                         // Add X_Location to the JSON to be record and send to the other board
    Loom.add_data("Y_Location", "MM", 0);                                         // Add Y_Location to the JSON to be record and send to the other board
    Loom.add_data("Z_Location", "MM", 0);                                         // Add Z_Location to the JSON to be record and send to the other board
    Loom.add_data("MaxSpeed", "Velocity", 0);                                     // Add MaxSpeed to the JSON to be record and send to the other board
    Loom.add_data("Spool_Rad_X", "Radius", 0);                                    // Add Spool_Rad_X to the JSON to be record and send to the other board
    Loom.add_data("Spool_Rad_YZ", "Radius", 0);                                   // Add Spool_Rad_YZ to the JSON to be record and send to the other board
    Loom.add_data("GoTo", "B", 0);                                                // Add GoTo Boolean to the JSON to  be record and send to the other board
    Loom.add_data("Loop", "B", 0);                                                // Add Loop Boolean to the JSON to be record and send to the other board
    Loom.add_data("Reset", "B", 0);                                               // Add Reset Boolean to the JSON to be record and send to the other board
    Loom.add_data("Calibrate", "B", 0);                                           // Add Calibrate Boolean to the JSON to be record and send to the other board
    Loom.add_data("PeriodX", "Num", 0);                                           // Add PeriodX to the JSON to be record and send to the other board
    Loom.add_data("PeriodY", "Num", 0);                                           // Add PeriodY to the JSON to be record and send to the other board
    Loom.add_data("PeriodZ", "Num", 0);                                           // Add PeriodZ to the JSON to be record and send to the other board

}

void updateValues(){
    const JsonObject coordinates_json = Loom.internal_json(false);                // Open the JSON from the code
    const JsonArray contents = coordinates_json["contents"];                      // For simple syntax uses
    contents[1]["data"]["MM"] = X_Location;                                       // Update the X_Location value
    contents[2]["data"]["MM"] = Y_Location;                                       // Update the Y_Location value
    contents[3]["data"]["MM"] = Z_Location;                                       // Update the Z_Location value
    contents[4]["data"]["Velocity"] = MaxSpeed;                                   // Update the MaxSpeed value
    contents[5]["data"]["Radius"] = Spool_Rad_X;                                  // Update the Spool_Rad_X value
    contents[6]["data"]["Radius"] = Spool_Rad_YZ;                                 // Update the Spool_Rad_YZ value
    contents[7]["data"]["B"] = GoTo;                                              // Update the GoTo value
    contents[8]["data"]["B"] = Loop;                                              // Update the MaxSpeed value
    contents[9]["data"]["B"] = Reset;                                             // Update the Spool_Rad_X value
    contents[10]["data"]["B"] = Calibrate;                                        // Update the Spool_Rad_YZ value
    contents[11]["data"]["Num"] = PeriodX;                                        // Update the MaxSpeed value
    contents[12]["data"]["Num"] = PeriodY;                                        // Update the Spool_Rad_X value
    contents[13]["data"]["Num"] = PeriodZ;                                        // Update the Spool_Rad_YZ value
}
