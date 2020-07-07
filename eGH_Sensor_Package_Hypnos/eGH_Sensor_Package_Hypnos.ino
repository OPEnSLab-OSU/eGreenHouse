/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// This is the eGreenHouse Sensor Collector Package.
// This program will measure values of the following sensors 
// then log that data values into SD, and send the data value
// to the Hub over LoRa(Radio Communicator).

// CO2 Sensor: K30 
// Luminosity Sensor(Light Sensor): TSL2591
// Temperature & Relative Humidity Sensor: SHT31-D 
// Particle Sensor(Dust & Pollen): SDS-011

// Note: The SDS-011 sensor will not measure it because the main purpose of it will only 
// give accurate values for the K30 sensor for giving more air to it.

// Note: The SD card will log the data value in the file called eGreenHouse.csv.

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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Loom.h>                                                                     // Need to include the Loom Package into the program

#include "wiring_private.h"                                                           // This .h file is for the K30 sensor

#include "eGreenhouse.h"                                                              // Include the JSON Package constructor

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

  pinMode(5,OUTPUT);
  digitalWrite(5,LOW);
  pinMode(6, OUTPUT);
  digitalWrite(6,HIGH);
  
  Serial2.begin(9600);                                                                // Start the Serial Sensor for K30
  Loom.begin_serial(true);                                                            // Start the Serial over Loom
  Loom.parse_config(json_config);                                                     // Add the config.h file into the program
  Loom.print_config();                                                                // Print out the config file if it works  

                                                                                      // Assign pins 10 & 11 SERCOM functionality for K30 sensor
  pinPeripheral(11, PIO_SERCOM);
  pinPeripheral(12, PIO_SERCOM);
  
  Loom.K30().set_serial(&Serial2);                                                    // Set the K30 sensor using Loom (note that we need those previous step to use Loom
  
  LPrintln("\n ** eGreenHouse Sensor Package Ready ** ");                           // Indicating the user that setup function is complete

  warmUpTimer();                                                                      // This will run the warm up the K30 sensor for 6 minutes: check line 54
}


void loop() {                                                                         // Put your main code here, to run repeatedly: 
  Loom.measure();                                                                     // Measuring the Sensor value                   
  Loom.package();                                                                     // Create the data value as one package with its own package number
  Loom.display_data();                                                                // Display printed JSON formatted data on serial monitor
  Loom.SDCARD().log("eGreenhouse.csv");                                               // Log the data values (packages) into the file from SD Card

  Loom.LoRa().send(3);                                                                // Send the package to the board that its ID is 9 (This board is 3 (look at config, LoRa))
  //Loom.pause(5000);                                                                 // Loom will pause to take a break for 5 second of measuring and go back to loop()

  Loom.power_down();
  
  digitalWrite(5,HIGH);
  digitalWrite(6,LOW);
  pinMode(23,INPUT);
  pinMode(24,INPUT);
  pinMode(10, INPUT);

  Loom.sleep();
  
  digitalWrite(5,LOW);
  digitalWrite(6,HIGH);
  pinMode(10, OUTPUT);
  pinMode(23, OUTPUT);
  pinMode(24, OUTPUT);

  Loom.power_up();
}

void SERCOM1_Handler()  {                                                             // This function needs for K30
  Serial2.IrqHandler();
}