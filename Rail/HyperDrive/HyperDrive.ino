//This program will recieve data from eGreenHouse_Sensor_Collector program, save it on the SD Card, and send the data to the hub
//while the hyperdrive will move arounf(A.K.A.) it will move around 
//Check the Hub and the eGreenHouse_Sensor_Collector programs

#include <Loom.h>

const char* json_config =
#include "config.h"
;

LoomFactory<
  Enable::Internet::Disabled,                                   //For GoogleSheet in Wifi/Ethernet,we need to enabled it
  Enable::Sensors::Disabled,                                    //For GoogleSheet in Wifi/Ethernet,we need to enabled it
  Enable::Radios::Enabled,                                      //For Communcation between boards
  Enable::Actuators::Enabled,                                   //For Enabling Motors 
  Enable::Max::Disabled                                         //Never ever Enable this part: It will kill the program
> ModuleFactory{};

LoomManager Loom{ &ModuleFactory };                             //Having all of the properties of LoomFactory<> in a manager called Loom



void setup() {
  Loom.begin_serial(true);                                      //Starting Loom
  Loom.parse_config(json_config);                               //Getting Information in Config.h file
  Loom.print_config();                                          //Printing out the config.h information to make sure it is running correctly or not
 
  LPrintln("\n **HyperDrive Ready** ");                         //Indicating the user that setup function is complete
}

void loop() {    

  if(Loom.LoRa().receive_blocking(500000)){                       //You have to use LoRa Blocking rather than LoRa Recieve becauase Recieve must be run at the same time with Transmit folder
                                                                  //There is a wait time to recieve the data (1000 = 1 second)
    //Move the coordinates as the following                       //In this case, we will wait for 5 minutes
    //(No code for this right now)                                

    LPrintln("HyperDrive has been moved to its position!");
    Loom.LoRa().send(9);

    //If there is a case that the HyperDrive is unable to move, it will do the following statements below
    //LPrintln("Somehow unable to move, the location is either at the starting position or previous position.")
    //Send the message above rather than the coordinates.
    //Loom.LoRa().send(9);   

  }
  else{
    LPrintln("Communcation Issues with the Hub, Trying again...");
    }
}
