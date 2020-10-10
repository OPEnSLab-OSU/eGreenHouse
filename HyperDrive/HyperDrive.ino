/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// This is the HyperDrive Code.
// This program will get the coordinates from Hub_Tramsmit Code
// then move to that location with that speed and rotation.  
// then send only X, Y, Z Location, and HyperRail Movement Status to eGreenhouse Sensor Package.
//
// Author: Kenneth Kang & Liam Duncan
//
// While Kenneth Kang wrote the code for communication, Liam Duncan wrote the HyperRail Movement code. 
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <ArduinoJson.h>                                                                                       // Need to include for the JsonDocument
#include <Loom.h>                                                                                              // Need to include the Loom Package into the program


#include "HyperRail_Driver.h"

const char* json_config =                                                                                      // Include Configuration
#include "config.h"
;

int checker;                                                                                                   // Declare a global variable for checking th correct JSON package

LoomFactory<
  Enable::Internet::Disabled,                                                                                  // For GoogleSheet in Wifi/Ethernet,we need to enabled it
  Enable::Sensors::Disabled,                                                                                   // For measureing data 
  Enable::Radios::Enabled,                                                                                     // For Communcation between boards
  Enable::Actuators::Disabled,                                                                                 // For Enabling Motors & Relay (one-pin)
  Enable::Max::Disabled                                                                                        // Never ever Enable this part: It will kill the program
> ModuleFactory{};

LoomManager Loom{ &ModuleFactory };                                                                            // Having all of the properties of LoomFactory<> in a manager called Loom

void setup() {                                                                                                 // Put your setup code here, to run once:

  Serial.begin(9600);

    // set up interrupts and pins 

  // XA Bump Switches 
  pinMode(X0ABump, INPUT_PULLUP); 
  pinMode(XMaxABump, INPUT_PULLUP); 

  // XB Bump Switches 
  pinMode(X0BBump, INPUT_PULLUP); 
  pinMode(XMaxBBump, INPUT_PULLUP); 

  // Y axis Bump Switches 
  pinMode(Y0Bump, INPUT_PULLUP); 
  pinMode(YMaxBump, INPUT_PULLUP); 

  // Z Axis Bump Switches 
  pinMode(Z0Bump, INPUT_PULLUP); 
  pinMode(ZMaxBump, INPUT_PULLUP);  

  // XA Motor Control Pins
  pinMode(STEPXA, OUTPUT); 
  pinMode(DIRXA, OUTPUT); 

  // XB Motor Control Pins 
  pinMode(STEPXB, OUTPUT); 
  pinMode(DIRXB, OUTPUT); 

  // Y Motor Control Pins
  pinMode(STEPY, OUTPUT); 
  pinMode(DIRY, OUTPUT); 

  // Z Motor Control Pins
  pinMode(STEPZ, OUTPUT); 
  pinMode(DIRZ, OUTPUT); 

  // Set Max Speed For all Steppers 
  stepperX.setMaxSpeed(MaxSpeed); 
  stepperY.setMaxSpeed(MaxSpeed/4); 
  stepperZ.setMaxSpeed(MaxSpeed/4); 

  // Set the Acceleration for the stepper motors
  stepperX.setAcceleration(40); 
  stepperY.setAcceleration(80);
  stepperZ.setAcceleration(40);


  
 //  initialize all interrupts for Bump Switches 

 //attachInterrupt(digitalPinToInterrupt(X0ABump), X0A_ISR, FALLING);

 attachInterrupt(digitalPinToInterrupt(X0BBump), X0B_ISR, FALLING);

 attachInterrupt(digitalPinToInterrupt(XMaxABump), XMaxA_ISR, FALLING);

 attachInterrupt(digitalPinToInterrupt(XMaxBBump), XMaxB_ISR, FALLING);

 attachInterrupt(digitalPinToInterrupt(Y0Bump), Y0_ISR, FALLING);

 attachInterrupt(digitalPinToInterrupt(YMaxBump), YMax_ISR, FALLING);

 attachInterrupt(digitalPinToInterrupt(Z0Bump), Z0_ISR, FALLING);

 attachInterrupt(digitalPinToInterrupt(ZMaxBump), ZMax_ISR, FALLING); 


  X0AFlag = 0; 
  XmaxAFlag = 0; 
  X0BFlag = 0; 
  XmaxBFlag = 0; 
  Y0Flag = 0; 
  YMAXFlag = 0; 
  Z0Flag = 0; 
  ZMAXFlag = 0;
  
  Loom.begin_serial(true);                                                                                     // Start the Serial over Loom
  Loom.parse_config(json_config);                                                                              // Add the config.h file into the program
  Loom.print_config();                                                                                         // Print out the config file if it works 

  LPrintln("\n ** Hyper Ready ** ");                                                                           // Indicating the user that setup function is complete
}

void loop() {                                                                                                  // Put your main code here, to run repeatedly:

   if(Loom.LoRa().receive_blocking(10000)){                                                                    // If LoRa receive something, then start these statments
    const JsonObject coordinates_json = Loom.internal_json(false);                                             // Open the JSON from the code
    const JsonArray contents = coordinates_json["contents"];                                                   // For simple syntax uses
    s
    const char* checker = coordinates_json["id"]["name"];                                                      // Update the checker value
    
    if (strcmp(checker, "eGH") == 0){                                                                          // Check if the board got the right JSON, if not, then it will move the else statement
      
    LPrintln("Got the user input Coordinate values");                                                          // Tell the user that we got the correct JSON
    
    X_Location = contents[1]["data"]["MM"];                                                                    // Get the X_Location from JSON
    Y_Location = contents[2]["data"]["MM"];                                                                    // Get the Y_Location from JSON
    Z_Location = contents[3]["data"]["MM"];                                                                    // Get the Z_Location from JSON
    MaxSpeed = contents[4]["data"]["Velocity"];                                                                // Get the MaxSpeed from JSON
    Spool_Rad_X = contents[5]["data"]["Radius"];                                                               // Get the Spool_Rad_X from JSON
    Spool_Rad_YZ = contents[6]["data"]["Radius"];                                                              // Get the Spool_Rad_YZ from JSON
    Goto = contents[7]["data"]["B"];                                                                           // Get the GoTo from JSON
    looP = contents[8]["data"]["B"];                                                                           // Get the Loop from JSON
    Reset = contents[9]["data"]["B"];                                                                          // Get the Reset from JSON
    calibrate = contents[10]["data"]["B"];                                                                     // Get the Calibrate from JSON
    periodX = contents[11]["data"]["Num"];                                                                     // Get the PeriodX from JSON
    periodY = contents[12]["data"]["Num"];                                                                     // Get the PeriodY from JSON
    periodZ = contents[13]["data"]["Num"];                                                                     // Get the PeriodZ from JSON
    
    int xsteps = mmToSteps(X_Location, X_SPR, Spool_Rad_X, X_Micro );
    int ysteps = mmToSteps(Y_Location, YZ_SPR, Spool_Rad_YZ, YZ_Micro );
    int zsteps = mmToSteps(Z_Location, YZ_SPR, Spool_Rad_YZ, YZ_Micro );
//    if(Goto == 1){
//      GoTo(xsteps, ysteps, zsteps); 
//    }
//    if(looP == 1){
//      Loop(); 
//    }
//
////    if(Reset == 1 and calibrate == 1){
////      GoTo(X0_pos, Y0_pos, Z0_pos);   
////    }
//  // Serial.print(xAMove);
//
//  // checkInts(); 
//
//
// // X0AFlag = checkInts(X0AFlag, X0ABump, X0A_pos, stepperX, xMove);
//
//  // if rail is not calibrated then calibrate it 
////    if(calibrate == 1){
////      Calibrate(); 
////    }
    Loom.pause(1000);
    Loom.internal_json(true);                                                                                  // Clear the internal_json from Loom
    Loom.package();                                                                                            // Create the Meta data for the json
    Loom.add_data("X_Location", "MM", X_Location);                                                             // Add X_Location to the JSON to be record and send to the other board
    Loom.add_data("Y_Location", "MM", Y_Location);                                                             // Add Y_Location to the JSON to be record and send to the other board
    Loom.add_data("Z_Location", "MM", Z_Location);                                                             // Add Z_Location to the JSON to be record and send to the other board
    
    Loom.display_data();                                                                                       // Display the new JSON to send the Sensor Package
    
    Loom.LoRa().send(9);                                                                                       // Send the JSON to the Sensor Package to measure values
    }
    else{
      LPrintln("Failed to get the Coordinates");                                                               // If the coordinate json was not sent, then it will print out this message
    } 
  }
}
