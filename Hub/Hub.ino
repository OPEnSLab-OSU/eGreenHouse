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

  // The code below will be part of sending coordinates to the HyperDrive while recieve coordinates from the GUI

  if (Serial.available()){
    GetData();
    Loom.package();
    Loom.LoRa().send(6);
  }

  // The code below will be part of recieving package from the SensorPackage

  if(Loom.LoRa().receive_blocking(500000)){                                   //You have to use LoRa Blocking rather than LoRa Recieve becauase Recieve must be run at the same time with Transmit folder
                                                                              //There is a wait time to recieve the data (1000 = 1 second)
    LPrintln("Package Recieve, Ready to publish!")
    Loom.display_data();                                                      

    Loom.GoogleSheets().publish();                                            //The statement is self-explanatory that you are publishing the data on the Googlesheets
  }

  else{
    LPrintln("Package not Recieve anything, Trying again...")                          //In this case, we will wait for 5 minutes. If we don't get a package, then we will return this statment
  }
}

// From the New_main_with_Accelstepper.ino
void GetData()
{
  // create JSON object
  DynamicJsonDocument doc(200); 

  // receive Json String From Processing   
  String JsonStr = Serial.readString(); 
  // Serial.println(JsonStr); 

  // Deserialize Object
  DeserializationError err = deserializeJson(doc, JsonStr); 

  // Print Error and Return From function if deserialization failed
  if (err) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(err.c_str());  
    return;
  }

  // Pretty Serialization 
  serializeJsonPretty(doc, JsonStr); 

  // Store all Values from JSON file as Ints 
  int X_Location = doc["X_Location"]; 
  int Y_Location = doc["Y_Location"]; 
  int Z_Location = doc["Z_Location"]; 
  int MaxSpeed = doc["Velocity"]; 
  int Spool_Rad_X = doc["SpoolRadX"]; 
  int Spool_Rad_YZ = doc["SpoolRadYZ"]; 
  
  return; 
}