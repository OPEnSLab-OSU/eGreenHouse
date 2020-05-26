//This program will recieve data from eGreenHouse_Sensor_Collector program, save it on the SD Card, and send the data to the hub
//while the hyperdrive will move arounf(A.K.A.) it will move around 
//Check the Hub and the eGreenHouse_Sensor_Collector programs

#include <Loom.h>
#include <ArduinoJson.h>
#include <AccelStepper.h>

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

 // define the steps per revolution for X,Y and Z motors 
 #define X_SPR 1700
 #define YZ_SPR 400 

 // Use these variables to define your microstep values for the motors 
 // When there is nothing connected to the big easy driver pins the driver defaults to 1/16 step 
 #define YZ_Micro 1 // 1/16 step 
 #define X_Micro 1  // full step 


 // define all interrupt pins for bump switches
 #define X0ABump 1
 #define X0BBump 14

 #define XMaxABump 0
 #define XMaxBBump 6

 #define Y0Bump A4
 #define YMaxBump 20

 #define Z0Bump 21
 #define ZMaxBump 15

 // Define Motor Pins X A,B
 #define STEPXA 10
 #define DIRXA 11

 #define STEPXB 12
 #define DIRXB 13

 // Define Motor Pins for Y 
 #define STEPY 9
 #define DIRY 16

 // Define Motor Pins for Z
 #define STEPZ 17
 #define DIRZ 19

 // foreward and backward definitions
 #define FORWARD 0
 #define BACKWARD 1

 String JsonStr;   // Define a string object to receive JSON data from Processing GUI


 // Length of X,Y,Z axis in meters
 int X_Location = 0; 
 int Y_Location = 0; 
 int Z_Location = 0; 

 // velocity you would like the rail to move at
 int Velocity; 

 // variables to hold the spool radius for X, Y, Z axis;
 int Spool_Rad_X = 32; 
 int Spool_Rad_YZ = 32; 

 // Position of the back end of the rail set by the calibration function
 int XA0_pos; 
 int XB0_pos; 
 int Y0_pos; 
 int Z0_pos; 
 int X0_pos; 

 // Position of the front end of the rail set by the calibration function
 int XAMAX_pos; 
 int XBMAX_pos; 
 int YMAX_pos; 
 int ZMAX_pos; 
 int XMAX_pos;

 // Make flags for the interrupts 
 bool XA0Flag = false; 
 bool XAMAXFlag = false; 
 bool XB0Flag = false; 
 bool XBMAXFlag = false; 
 bool Y0Flag = false; 
 bool YMAXFlag = false; 
 bool Z0Flag = false; 
 bool ZMAXFlag = false; 

 // Calibration Speeds
 int XCalSpeed = 600; 
 int YZCalSpeed = 200; 

 // Define the max stepper speed in Steps/second
 int MaxSpeed = 2500; 

 // calibration has happened or not
 bool calibrated = false; 

 // ensure that the interrupts do not trigger when switches are released 
 int Y0_Count = 0; 
 int XA0_Count = 0; 
 int XB0_Count = 0; 
 int Z0_Count = 0; 

 int YMAX_Count = 0;
 int XAMAX_Count = 0;
 int XBMAX_Count = 0; 
 int ZMAX_Count = 0; 


void setup() {
  Serial.begin(9600);

  Loom.begin_serial(true);                                      //Starting Loom
  Loom.parse_config(json_config);                               //Getting Information in Config.h file
  Loom.print_config();                                          //Printing out the config.h information to make sure it is running correctly or not
  
    // set up interrupts and pins 

  // XA Bump Switches 
  pinMode(X0ABump, INPUT); 
  pinMode(XMaxABump, INPUT); 

  // XB Bump Switches 
  pinMode(X0BBump, INPUT); 
  pinMode(XMaxBBump, INPUT); 

  // Y axis Bump Switches 
  pinMode(Y0Bump, INPUT); 
  pinMode(YMaxBump, INPUT); 

  // Z Axis Bump Switches 
  pinMode(Z0Bump, INPUT); 
  pinMode(ZMaxBump, INPUT);  

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

 // X0A Interrupt 
 Loom.InterruptManager().register_ISR(X0ABump, X0A_ISR, FALLING, ISR_Type::IMMEDIATE);

 // X0B Interrupt 
 Loom.InterruptManager().register_ISR(X0BBump, X0B_ISR, FALLING, ISR_Type::IMMEDIATE);

 // XMaxA Interrupt 
 Loom.InterruptManager().register_ISR(XMaxABump, XMaxA_ISR, FALLING, ISR_Type::IMMEDIATE);

 // XMaxB Interrupt 
 Loom.InterruptManager().register_ISR(XMaxBBump, XMaxB_ISR, FALLING, ISR_Type::IMMEDIATE);

 // Y0 Interrupt 
 Loom.InterruptManager().register_ISR(Y0Bump, Y0_ISR, FALLING, ISR_Type::IMMEDIATE);

 // YMax Interrupt 
 Loom.InterruptManager().register_ISR(YMaxBump, YMax_ISR, FALLING, ISR_Type::IMMEDIATE);

 // Z0 Interrupt 
 Loom.InterruptManager().register_ISR(Z0Bump, Z0_ISR, FALLING, ISR_Type::IMMEDIATE);

 // ZMax Interrupt 
 Loom.InterruptManager().register_ISR(ZMaxBump, ZMax_ISR, FALLING, ISR_Type::IMMEDIATE);

  LPrintln("\n **HyperDrive Ready** ");                         //Indicating the user that setup function is complete
}

void loop() {    

  if(Loom.LoRa().receive_blocking(500000)){                       //You have to use LoRa Blocking rather than LoRa Recieve becauase Recieve must be run at the same time with Transmit folder
                                                                  //There is a wait time to recieve the data (1000 = 1 second)
    //Move the coordinates as the following                       //In this case, we will wait for 5 minutes
    
    // Set Max Speed For all Steppers 
    stepperX.setMaxSpeed(MaxSpeed); 
    stepperY.setMaxSpeed(MaxSpeed/2); 
    stepperZ.setMaxSpeed(MaxSpeed/2);                       
            
    int xsteps = mmToSteps(X_Location, X_SPR, Spool_Rad_X, X_Micro );
    int ysteps = mmToSteps(Y_Location, YZ_SPR, Spool_Rad_YZ, YZ_Micro );
    int zsteps = mmToSteps(Z_Location, YZ_SPR, Spool_Rad_YZ, YZ_Micro );

    GoTo(xsteps, ysteps, zsteps);                               

    LPrintln("HyperDrive has been moved to its position!");
    Loom.LoRa().send(9);



  }
  else{
    LPrintln("Communcation Issues with the Hub, Trying again...");
    }
}

// Everything below are the functions or variables

int mmToSteps(double mm, int steps_per_revolution, double belt_radius, int micro) {

  
        return (int) round((mm*micro)/(2*3.14*belt_radius) * steps_per_revolution);
    }


// Function Gets data from serial port GUI, 
// turns it into a JSON object, and sets int  
// values 

// One stepper Motor Step 
void onestep(int dir, int stepPin, int dirPin) {

// set the direction to turn
   if (dir == 1){ 
      digitalWrite(dirPin, HIGH);
   }
   else {
      digitalWrite(dirPin, LOW);
   }

   digitalWrite(stepPin, HIGH);
   delayMicroseconds(60);
   digitalWrite(stepPin, LOW);
   delayMicroseconds(60); 
}

// Functions for Stepper Motor Stepping 

void forwardXA() {
  onestep(FORWARD, STEPXA, DIRXA);
}

void backwardXA() {
  onestep(BACKWARD, STEPXA, DIRXA);
}

void forwardXB() {
  onestep(FORWARD, STEPXB, DIRXB);
}

void forwardX()
{
  onestep(FORWARD, STEPXA, DIRXA);
  onestep(FORWARD, STEPXB, DIRXB);
}

void backwardX()
{
  onestep(BACKWARD, STEPXA, DIRXA);
  onestep(BACKWARD, STEPXB, DIRXB);
}

void backwardXB() {
  onestep(BACKWARD, STEPXB, DIRXB);
}

void forwardY() {
  onestep(FORWARD, STEPY, DIRY);
}

void backwardY() {
  onestep(BACKWARD, STEPY, DIRY);
}


void forwardZ() {
  onestep(FORWARD, STEPZ, DIRZ);
}

void backwardZ() {
  onestep(BACKWARD, STEPZ, DIRZ);
}


   // construct Motor objects 
   AccelStepper stepperX(forwardX, backwardX);
   AccelStepper stepperY(forwardY, backwardY);
   AccelStepper stepperZ(forwardZ, backwardZ); 


 // put interrupt functions here 

  void X0A_ISR()
 {
   stepperX.stop(); 

   delay(5); 
   XA0_Count++; 

   if(XA0_Count == 1)
   {
   XA0_pos = stepperX.currentPosition(); 
   XA0Flag = true; 
   Serial.println("XA0 Flag"); 
   Serial.println(XA0_pos);
   }
   else
   XA0_Count = 0; 
 }



 void Y0_ISR()
 {
   stepperY.stop(); 

   delay(5); 
   Y0_Count++; 

   if(Y0_Count == 1)
   {
   Y0_pos = stepperY.currentPosition(); 
   //stepperY.currentPosition(); 
   Y0Flag = true; 
   Serial.println("Y0 Flag"); 
   Serial.println(Y0_pos);
   }
  else
   Y0_Count = 0; 
  
 }

 void X0B_ISR()
 {
   stepperX.stop(); 

   delay(5); 
   XB0_Count++; 

   if(XB0_Count == 1)
   {
   XB0_pos = stepperX.currentPosition(); 
   //stepperXB.currentPosition(); 
   XB0Flag = true; 
   Serial.println("XB0 Flag"); 
   Serial.println(XB0_pos);
   }
   else
   XB0_Count = 0; 
 }


// ISR for Z0 
void Z0_ISR()
{
  stepperZ.stop(); 

  delay(5);   // debounce 
  Z0_Count++;   // count to avoid double triggering 

  if(Z0_Count == 1) // if pressed 
  {
  Z0_pos = stepperZ.currentPosition(); 
  // stepperZ.currentPosition();  // set current position 
  Z0Flag = true;                        // set flag 
  Serial.println("Z0 Flag");            // print things for testing 
  Serial.println(Z0_pos);
  }
  else                                  // reset counter when interrupt is triggered on release of switch 
  Z0_Count = 0;  
}



void XMaxA_ISR()
{
  stepperX.stop(); 

  delay(5); 
  XAMAX_Count++; 

  if(XAMAX_Count == 1)
  {
  XAMAX_pos = stepperX.currentPosition(); 
  //stepperXA.currentPosition(); 
  XAMAXFlag = true; 
  Serial.println("XAMAX Flag"); 
  Serial.println(XAMAX_pos);
  }
  else
  XAMAX_Count = 0; 
}



void YMax_ISR()
{
  stepperY.stop(); 

  delay(5); 
  YMAX_Count++; 

  if(YMAX_Count == 1)
  {
  YMAX_pos = stepperY.currentPosition(); 
  //stepperY.currentPosition(); 
  YMAXFlag = true; 
  Serial.println("YMAX Flag"); 
  Serial.println(YMAX_pos);
  }
  else
  YMAX_Count = 0; 
}



void XMaxB_ISR()
{
  stepperX.stop(); 
  
  delay(5); 
  XBMAX_Count++; 

  if(XBMAX_Count == 1)
  {
  XBMAX_pos = stepperX.currentPosition(); 
  //stepperXB.currentPosition(); 
  XBMAXFlag = true; 
  Serial.println("XBMAX Flag"); 
  Serial.println(XBMAX_pos);
  }
  else
  XBMAX_Count = 0; 
}



void ZMax_ISR()
{
  stepperZ.stop(); 

  delay(5);   // debounce 
  ZMAX_Count++;   // count to avoid double triggering 

  if(ZMAX_Count == 1) // if pressed 
  {
  ZMAX_pos = stepperZ.currentPosition(); 
  //stepperZ.currentPosition();  // set current position 
  ZMAXFlag = true;                        // set flag 
  Serial.println("ZMAX Flag");            // print things for testing 
  Serial.println(ZMAX_pos);
  }
  else                                  // reset counter when interrupt is triggered on release of switch 
  ZMAX_Count = 0;  
}

void GoTo(int x, int y, int z)
{

 // Use MoveTo Function to set desired position 

 stepperX.moveTo(x); 
 stepperY.moveTo(y);
 stepperZ.moveTo(z);

 int i = 1, j = 1, k = 1; 
 while(i == 1 or j == 1 or k == 1)
 {
   i =  stepperX.run(); 
   j =  stepperY.run();
   k =  stepperZ.run();
 }

  return; 
}


// Function to calibrate all axis and set MAX and 
// 0 points for all axis individually 
void Calibrate()
{
  
    while(!XA0Flag or !XAMAXFlag or !XB0Flag or !XBMAXFlag or !Y0Flag or !YMAXFlag or !Z0Flag or !ZMAXFlag)
    {

      // XA 0 point calibration 
      if(!XA0Flag)
      {   
      stepperX.setSpeed(-XCalSpeed); 
      stepperX.runSpeed(); 
      }

      // XB 0 point calibration 
      if(!XB0Flag)
      {
        stepperX.setSpeed(-XCalSpeed); 
        stepperX.runSpeed(); 
      }

      // Y 0 point calibration 
      if(!Y0Flag)
      {
        stepperY.setSpeed(-YZCalSpeed);
        stepperY.runSpeed();
       }


      // Z 0 point calibration 
      if(!Z0Flag)
      {
       stepperZ.setSpeed(-YZCalSpeed); 
       stepperZ.runSpeed(); 
      }

     // XA Max Point calibration 
      if(XA0Flag and !XAMAXFlag)
      {
        stepperX.setSpeed(XCalSpeed); 
        stepperX.runSpeed();
      }

      // XB Max Point Calibration 
      if(XB0Flag and !XBMAXFlag)
      {
        stepperX.setSpeed(XCalSpeed); 
        stepperX.runSpeed();
      }

      // Y Max Point Calibration 
      if(Y0Flag and !YMAXFlag)
      {
        stepperY.setSpeed(YZCalSpeed); 
        stepperY.runSpeed();
      }

      // Z max point calibration
      if(Z0Flag and !ZMAXFlag)
      {

        stepperZ.setSpeed(YZCalSpeed); 
        stepperZ.runSpeed();
      }

    }

    XA0Flag = false; 
    XB0Flag = false; 
    XAMAXFlag = false; 
    XBMAXFlag = false; 
    Y0Flag = false; 
    YMAXFlag = false; 
    Z0Flag = false; 
    ZMAXFlag = false; 

    // set calibration flag 
    calibrated = true; 
   
}
