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

// If control from the GUI recieve, then send command that to power up and measure data from the sensor package

  //if(Loom.LoRa().receive_blocking(5000)){ Loom.LoRa.send_impl(1,3)}
  //Loom.LoRa().send_impl(1,3);
  if(Loom.LoRa().receive_blocking(5000)){                       //You have to use LoRa Blocking rather than LoRa Recieve becauase Recieve must be run at the same time with Transmit folder
                                                                //There is a wait time to recieve the data (1000 = 1 second)
//Logging data on the SD Card
//Loom.SDCARD().log();                                          //You can either use this statement that will generate a file name "datafile.csv"
                                                                //But you can make your own file name like mine
  Loom.SDCARD().log("Ken.csv");                                 //I recommend using it to identify the difference the data/organization

//Communcating with the other borad  
  Loom.LoRa().send(9);                                          //You are sending Data to a board that as the address of 9 in the parameters
                                                                //When you are writing the Receive code, unless you are turing both board at the same time,
                                                                //You must need to write the Receive Blocking code rather than normal Receive

  }
}
