 // File: HyperRail_Main_New.ino
 // Name: Liam Duncan 
 //        Made for OPEnS Lab 
 // Date: 1/21/2020
 // Missing the actual code for moving the hyperRail

 #include <ArduinoJson.h>
 #include <Loom.h>

 const char* json_config =
 #include "config.h"
 ;

 LoomFactory<
  Enable::Internet::Disabled,
  Enable::Sensors::Disabled,
  Enable::Radios::Enabled,
  Enable::Actuators::Disabled,
  Enable::Max::Disabled
> ModuleFactory{};

LoomManager Loom{ &ModuleFactory };

void setup() {

  Loom.begin_serial(true);
  Loom.parse_config(json_config);
  Loom.print_config();

  LPrintln("\n ** Hyper Ready ** ");  
}

// main Loop 
void loop() {

   if(Loom.LoRa().receive_blocking(5000)){
    
    Loom.display_data();

    DynamicJsonDocument doc(400);
    
    const JsonObject coordinates_json = Loom.internal_json(false);

    const JsonArray contents = coordinates_json["contents"];
        
    int X_Location = contents[0]["data"]["MM"]; 
    int Y_Location = contents[1]["data"]["MM"]; 
    int Z_Location = contents[2]["data"]["MM"]; 
    int MaxSpeed = contents[3]["data"]["Velocity"]; 
    int Spool_Rad_X = contents[4]["data"]["Radius"]; 
    int Spool_Rad_YZ = contents[5]["data"]["Radius"];

    doc.clear();

    LPrintln(X_Location);
    
    Loom.add_data("X_Locatiton", "MM", X_Location);
    Loom.add_data("Y_Locatiton", "MM", Y_Location);
    Loom.add_data("Z_Locatiton", "MM", Z_Location);
    
    Loom.display_data();
   
    Loom.LoRa().send(9);
    
   }

 }
