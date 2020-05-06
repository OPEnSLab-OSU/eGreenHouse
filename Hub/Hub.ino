//This program will get data from the HyperDrive program and publish it on GoogleSheets
//Check out SensorPackage and HyperDrive programs
//Also, If you want to change the googlesheet publish address, please check the googlesheet parts in the config

#include <Loom.h>

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

  // Send coordinates values as a JSON package to the HyperDrive
  // Loom.LoRa().send(6);


  // The code below will be part of recieving package from the SensorPackage

  if(Loom.LoRa().receive_blocking(500000)){                                   //You have to use LoRa Blocking rather than LoRa Recieve becauase Recieve must be run at the same time with Transmit folder
                                                                              //There is a wait time to recieve the data (1000 = 1 second)
    LPrintln("Package Recieve, Ready to publish!")
    Loom.display_data();                                                      

    Loom.GoogleSheets().publish();                                            //The statement is self-explanatory that you are publishing the data on the Googlesheets
  }

  else{
    LPrintln("Package not Recieve, Trying again...")                          //In this case, we will wait for 5 minutes. If we don't get a package, then we will return this statment
  }
}
