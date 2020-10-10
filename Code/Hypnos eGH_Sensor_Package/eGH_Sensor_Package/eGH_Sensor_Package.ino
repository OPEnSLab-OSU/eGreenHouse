  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// This is the eGreenHouse Sensor Package.
// This program will get the coordinates from HyperDrive Code
// then measure values of the following sensors 
// then log that data values into SD, and send the data value
// to the Hub over LoRa(Radio Communicator).
//
// CO2 Sensor: K30 
// Luminosity Sensor(Light Sensor): TSL2591
// Temperature & Relative Humidity Sensor: SHT31-D 
//
// In that file, the order will be the following:
// A. Device Name
// B. Device Number
// C. Date from RTC for UTC
// D. Time from RTC for UTC
// E. Package Number
// F. Temperature in celsius from SHT-031(there is =-0.3 celsius error)
// G. Humanity in % from SHT-031(there is +- 2% error) 
// H. Visible Spectrum Light in nm from TSL2591 
// I. Infrered Lightin nm from TSL2591 
// J. Full Spectrum Light in nm from TSL2591
// K. CO2 value in ppm
// L. Location in mm
//
// Author: Kenneth Kang
//
//
// Somehow the code sometimes doesn't work properly, even though the code works completely fine. 
// Therefore, you might want to complie the Basic Loom code before compling this code
//
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Loom.h>                                                                     // Need to include the Loom Package into the program

#include "wiring_private.h"                                                           // This .h file is for the K30 sensor
                                                                                      
const char* json_config =                                                             // Include Configuration
#include "config.h"
;

LoomFactory<
  Enable::Internet::Disabled,                                                         // For GoogleSheet in Wifi/Ethernet,we need to enabled it
  Enable::Sensors::Enabled,                                                           // For measureing data 
  Enable::Radios::Enabled,                                                            // For Communcation between boards
  Enable::Actuators::Disabled,                                                        // For Enabling Motors & Relay (one-pin)
  Enable::Max::Disabled                                                               // Never ever Enable this part: It will kill the program
> ModuleFactory{};

LoomManager Loom{ &ModuleFactory };                                                   // Having all of the properties of LoomFactory<> in a manager called Loom

Uart Serial2 = Uart(&sercom1, 13, 11, SERCOM_RX_PAD_1, UART_TX_PAD_0);                // Create Serial SERCOM for K30 Sensor: RX pin 13, TX pin 11
 
void warmUpTimer(){                                                                   // This function is a timer to warm up the K30 sensor to get accurate measurements
  
  LPrintln("\n ** Set up 6 minutes Warm Up time to get accurate measurements ** ");

  for(int timePassed = 1; timePassed < 7; timePassed++){                              // By pausing Loom, it will not measure CO2 value for 6 minutes
    Loom.pause(60000);                                                                // The max is only 1 min for pause, we loop it for 6 times to make it 6 minutes
    LPrint(timePassed);                                                               // Knowing the User that how many minutes have been passed
    LPrint(" minute(s) passed!");
    LPrint("\n");
  }
  
  LPrintln("\n ** Ready to Measure ** ");                                             // Let the user know that eGH_Sensor_Package is ready for measurements
}


void setup() {                                                                        // Put your setup code here, to run once:
  
  Serial2.begin(9600);                                                                // Start the Serial Sensor for K30
   
  // Needs to be done for Hypno Board
  pinMode(5, OUTPUT);                                                                 // Enable control of 3.3V rail 
  pinMode(6, OUTPUT);                                                                 // Enable control of 5V rail 

  //See Above
  digitalWrite(5, LOW);                                                               // Enable 3.3V rail
  digitalWrite(6, HIGH);                                                              // Enable 5V rail
  
  
  Loom.begin_serial(true);                                                            // Start the Serial over Loom
  Loom.parse_config(json_config);                                                     // Add the config.h file into the program
  Loom.print_config();                                                                // Print out the config file if it works

                                                                                      // Assign pins 11 & 13 SERCOM functionality for K30 sensor
  pinPeripheral(11, PIO_SERCOM);
  pinPeripheral(13, PIO_SERCOM);
  
  Loom.K30().set_serial(&Serial2);                                                    // Set the K30 sensor using Loom (note that we need those previous step to use Loom
 
  LPrintln("\n ** eGreenHouse Sensor Package Ready ** ");                             // Indicating the user that setup function is complete

  warmUpTimer();                                                                      // This will run the warm up the K30 sensor for 6 minutes: check line 56
}
void loop() {                                                                         // Put your main code here, to run repeatedly:
 
  if(Loom.LoRa().receive_blocking(10000)){
         
      const JsonObject coordinates_json = Loom.internal_json(false);                 // Create a new JsonObject that was received from the HyperRail

      const JsonArray contents = coordinates_json["contents"];                       // Create a JsonArray from the JSON 
      
      const char* checker = coordinates_json["id"]["name"];                          // checker value
      if(strcmp(checker, "eGH_Hyper") == 0){                                         // Checking if we get the correct JSON Message

        int Location = contents[1]["data"]["MM"];                                    // Store Location value from the JSON

        Loom.measure();                                                              // Measure Sensor and Time 
        Loom.package();                                                              // Make them into a new JSON
        Loom.add_data("Location", "MM", Location);                                   // Add Location to be record and send to the other board
                
        Loom.display_data();                                                         // Display printed JSON formatted data on serial monitor
        Loom.SDCARD().log();                                                         // Log the data values (packages) into the file from SD Card

        Loom.LoRa().send(3);                                                         // Send back to the Hub
      }
      else{
        LPrintln("Incorrect Message, Retrying again");                               // If the checker fails, then it will return this message, which it is fine 
     }
  }
}

void SERCOM1_Handler()  {                                                             // This function needs for K30
  Serial2.IrqHandler();
}
