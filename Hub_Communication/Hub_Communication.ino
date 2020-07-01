//This program will get data from the HyperDrive program and publish it on GoogleSheets
//Check out SensorPackage and HyperDrive programs
//Also, If you want to change the googlesheet publish address, please check the googlesheet parts in the config

#include <Loom.h>
#include <ArduinoJson.h>


#include "eGreenhouseJSON.h"

//Basic Setup for Loom
const char* json_config = 
#include "config.h"
;

LoomFactory<
  Enable::Internet::All,                                                      //For GoogleSheet in Wifi/Ethernet,we need to enabled it
  Enable::Sensors::Disabled,                                                  //For getting sensor data: We don't need it for this program
  Enable::Radios::Enabled,                                                    //For Communcation between boards
  Enable::Actuators::Disabled,                                                //For Motors (It will be part in the Hyperdrive)
  Enable::Max::Disabled                                                       //Never ever Enable this part: It will kill the program
> ModuleFactory{};

LoomManager Loom{ &ModuleFactory };                                           //Having all of the properties of LoomFactory<> in a manager called Loom

void setup() {                                                                //Put your setup code here, to run once:
  Loom.begin_serial(true);                                                    //Starting Loom
  Loom.parse_config(json_config);                                             //Getting Information in Config.h file
  Loom.print_config();                                                        //Printing out the config.h information to make sure it is running correctly or not

  LPrintln("\n ** Hub Ready ** ");                                            //Indicating the user that setup function is complete

}

void loop() {                                                                 //Put your main code here, to run repeatedly:

  // Read Json Object from Processing
  if(Serial.available()){
    DynamicJsonDocument doc(400);
    String JsonStr = Serial.readString();

    DeserializationError err = deserializeJson(doc, JsonStr);

    if (err){
      Serial.print(F("deserializeJson() failed with code "));
      Serial.println(err.c_str());

      return ;
    }

    serializeJsonPretty(doc, JsonStr);

    

    int X_Location = doc["X_Location"]; 
    int Y_Location = doc["Y_Location"]; 
    int Z_Location = doc["Z_Location"]; 
    int MaxSpeed = doc["Velocity"]; 
    int Spool_Rad_X = doc["SpoolRadX"]; 
    int Spool_Rad_YZ = doc["SpoolRadYZ"];

    doc.clear();
    
    Loom.add_data("X_Locatiton", "MM", X_Location);
    Loom.add_data("Y_Locatiton", "MM", Y_Location);
    Loom.add_data("Z_Locatiton", "MM", Z_Location);
    Loom.add_data("MaxSpeed", "Velocity", MaxSpeed);
    Loom.add_data("Spool_Rad_X", "Radius", Spool_Rad_X);
    Loom.add_data("Spool_Rad_YZ", "Radius", Spool_Rad_YZ);

    Loom.display_data();
    Loom.LoRa().send(6);
    
  }
  
  eGreenhouse_Base in_data;
  if(Loom.LoRa().receive_blocking_raw(in_data.raw, sizeof(in_data.raw), 1000)){
    JsonObject internal_json = Loom.internal_json(true);
    struct_to_json(in_data, internal_json);
    Loom.display_data();
    Loom.GoogleSheets().publish();
  }
}
