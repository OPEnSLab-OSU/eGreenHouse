/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// This is the eGreenHouse Sensor Package.
// This program will get the coordinates from HyperDrive Code
// then measure values of the following sensors 
// then log that data values into SD, and send the data value
// to the Hub over LoRa(Radio Communicator).

// CO2 Sensor: K30 
// Luminosity Sensor(Light Sensor): TSL2591
// Temperature & Relative Humidity Sensor: SHT31-D 

// In that file, the order will be the following:
// A. Device Name
// B. Device Number
// C. Date from RTC
// D. Time from RTC
// E. Package Number
// F. Temperature in celsius from SHT-031(there is =-0.3 celsius error)
// G. Humanity in % from SHT-031(there is +- 2% error) 
// H. Visible Spectrum Light in nm from TSL2591 
// I. Infrered Lightin nm from TSL2591 
// J. Full Spectrum Light in nm from TSL2591
// K. CO2 value in ppm
// L. X_Location in mm
// M. Y_Location in mm
// N. Z_Location in mm

// Author: Kenneth Kang

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Loom.h>                                                                     // Need to include the Loom Package into the program

#include "wiring_private.h"                                                           // This .h file is for the K30 sensor

#include "eGreenhouseJSON.h"                                                          // Include the JSON Package constructor
                                                                                      // Include Configuration
const char* json_config =
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

Uart Serial2 = Uart(&sercom1, 12, 11, SERCOM_RX_PAD_3, UART_TX_PAD_0);                // Create Serial SERCOM for K30 Sensor: RX pin 12, TX pin 11
 
void warmUpTimer(){                                                                   // This function is a timer to warm up the K30 sensor to get accurate measurements
  
  LPrintln("\n ** Set up 6 minutes Warm Up time to get accurate measurements ** ");

  for(int timePassed = 1; timePassed < 7; timePassed++){                              // By pausing Loom, it will not measure CO2 value for 6 minutes
    Loom.pause(60000);                                                                // The max is only 1 min for pause, we loop it for 6 times to make it 6 minutes
    LPrint(timePassed);                                                               // Knowing the User that how many minutes have been passed
    LPrint(" minute(s) passed!");
    LPrint("\n");
  }
  
  LPrintln("\n ** Ready to Measure ** ");
}


void setup() {                                                                        // Put your setup code here, to run once:

  Serial2.begin(9600);                                                                // Start the Serial Sensor for K30
  Loom.begin_serial(true);                                                            // Start the Serial over Loom
  Loom.parse_config(json_config);                                                     // Add the config.h file into the program
  Loom.print_config();                                                                // Print out the config file if it works  

                                                                                      // Assign pins 10 & 11 SERCOM functionality for K30 sensor
  pinPeripheral(11, PIO_SERCOM);
  pinPeripheral(12, PIO_SERCOM);
  
  Loom.K30().set_serial(&Serial2);                                                    // Set the K30 sensor using Loom (note that we need those previous step to use Loom

  LPrintln("\n ** eGreenHouse Sensor Collector Ready ** ");                           // Indicating the user that setup function is complete

  warmUpTimer();                                                                      // This will run the warm up the K30 sensor for 6 minutes: check line 56
}

int checker = 11;

void loop() {                                                                         // Put your main code here, to run repeatedly:
 
  if(Loom.LoRa().receive_blocking(10000)){
      
      Loom.display_data();
      
      const JsonObject coordinates_json = Loom.internal_json(false);                    // Create a new JsonObject that was received from the HyperRail

      const JsonArray contents = coordinates_json["contents"];                          // Create a JsonArray from the JSON 
      
      checker = contents[0]["data"]["B"];
      LPrintln(checker);
      if(checker == 2){

        int X_Location = contents[1]["data"]["MM"];
        int Y_Location = contents[2]["data"]["MM"];
        int Z_Location = contents[3]["data"]["MM"];

        Loom.measure();
        Loom.package();
        Loom.add_data("X_Locatiton", "MM", X_Location);                                 // Add X_Location to be record and send to the other board
        Loom.add_data("Y_Locatiton", "MM", Y_Location);                                 // Add Y_Location to be record and send to the other board
        Loom.add_data("Z_Locatiton", "MM", Z_Location);                                 // Add Z_Location to be record and send to the other board
        Loom.add_data("Hyper_Passes", "B", 1);
                
        Loom.display_data();                                                            // Display printed JSON formatted data on serial monitor
        Loom.SDCARD().log();                                                            // Log the data values (packages) into the file from SD Card

        eGreenhouse_Base out_struct;                                                    // Create a new out_struct to send large size content over LoRa
        const JsonObjectConst internal_data = Loom.internal_json(false);                // Create a new Json Object with the Sensor values
        eGH_json_to_struct(internal_data, out_struct);                                      // Use that new Json to convert to Struct
        Loom.LoRa().send_raw(out_struct.raw, sizeof(out_struct.raw), 12);                // Send out the Struct Data to the other Board: Check out eGreenhouse.cpp and eGreenhouse.h
      }
      else{
        LPrintln("Failed to get the correct message");
     }
  }
}

void SERCOM1_Handler()  {                                                             // This function needs for K30
  Serial2.IrqHandler();
}
