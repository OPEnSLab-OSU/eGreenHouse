 // File: HyperRail_Main_New.ino
 // Name: Liam Duncan 
 //        Made for OPEnS Lab 
 // Date: 1/21/2020
 // Description: Code to Drive the HyperRail, Interface with Processing GUI, and Receive data from eGreenhouse

 #include <ArduinoJson.h>
 #include <AccelStepper.h>
 #include <Loom.h>

 const char* json_config =
 #include "config.h"
 ;

 LoomFactory<
  Enable::Internet::Disabled,
  Enable::Sensors::Disabled,
  Enable::Radios::Enabled,
  Enable::Actuators::Disabled,
  Enable::Max::Disabled
> ModuleFactory{};

LoomManager Loom{ &ModuleFactory };

 // define the steps per revolution for X,Y and Z motors 
 #define X_SPR 1700
 #define YZ_SPR 400 

 // Use these variables to define your microstep values for the motors 
 // When there is nothing connected to the big easy driver pins the driver defaults to 1/16 step 
 #define YZ_Micro 1 // 1/16 step 
 #define X_Micro 1  // full step 


 // define all interrupt pins for bump switches
 int X0ABump = 1;
 int X0BBump = 14;

 int XMaxABump = 0;
 int XMaxBBump = 6;

 int Y0Bump = 18;
 int YMaxBump = 20;

 int Z0Bump = 21;
 int ZMaxBump = 15;

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

 bool xMove = true; 
 bool yMove = true; 
 bool zMove = true; 

 int xLoopPeriod = 8000; 
 int yLoopPeriod = 8000; 
 int zLoopPeriod = 4000; 

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
 int X0A_pos; 
 int X0B_pos; 
 int Y0_pos; 
 int Z0_pos; 
 int X0_pos; 

 // Position of the front end of the rail set by the calibration function
 int XmaxA_pos; 
 int XmaxB_pos; 
 int Ymax_pos; 
 int Zmax_pos; 
 int Xmax_pos;

 // Make flags for the interrupts 
 volatile int XA0Flag = 0; 
 bool XmaxAFlag = false; 
 bool X0BFlag = false; 
 bool XmaxBFlag = false; 
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
  onestep(BACKWARD, STEPXA, DIRXA);
  onestep(FORWARD, STEPXB, DIRXB);
}

void backwardX()
{
  onestep(FORWARD, STEPXA, DIRXA);
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
   Serial.println("A0");
   XA0Flag = 1; 
   xMove = 0; 
   delay(10);
 }



 void Y0_ISR()
 {
   Y0Flag = true;
 }

 void X0B_ISR()
 {
   X0BFlag = true; 
 }


// ISR for Z0 
void Z0_ISR()
{
  Z0Flag = true; 
}



void XMaxA_ISR()
{
  XmaxAFlag = true; 
}



void YMax_ISR()
{
  YMAXFlag = true; 
}



void XMaxB_ISR()
{
  XmaxBFlag = true; 
}



void ZMax_ISR()
{
  ZMAXFlag = true; 
}
 


void setup() {

  Serial.begin(9600);

  Loom.begin_serial(true);
  Loom.parse_config(json_config);
  Loom.print_config();

  Serial.print("Arduino is Ready!"); 

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

 attachInterrupt(digitalPinToInterrupt(X0ABump), X0A_ISR, FALLING);

 attachInterrupt(digitalPinToInterrupt(X0BBump), X0B_ISR, FALLING);

 attachInterrupt(digitalPinToInterrupt(XMaxABump), XMaxA_ISR, FALLING);

 attachInterrupt(digitalPinToInterrupt(XMaxBBump), XMaxB_ISR, FALLING);

 attachInterrupt(digitalPinToInterrupt(Y0Bump), Y0_ISR, FALLING);

 attachInterrupt(digitalPinToInterrupt(YMaxBump), YMax_ISR, FALLING);

 attachInterrupt(digitalPinToInterrupt(Z0Bump), Z0_ISR, FALLING);

 attachInterrupt(digitalPinToInterrupt(ZMaxBump), ZMax_ISR, FALLING);
 

}


int mmToSteps(double mm, int steps_per_revolution, double belt_radius, int micro) {

        // Serial.print("mm = ");
        // Serial.println(mm); 

        // Serial.print("steps per rev = "); 
        // Serial.println(steps_per_revolution); 

        // Serial.print("Radius = "); 
        // Serial.println(belt_radius); 
  
        return (int) round((mm*micro)/(2*3.14*belt_radius) * steps_per_revolution);
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
    if(XA0Flag or XmaxAFlag or X0BFlag or XmaxBFlag or Y0Flag or YMAXFlag or Z0Flag or ZMAXFlag)
    {
      //Serial.println("Break");
      break;
    }

    if(xMove)
    i =  stepperX.run(); 

    if(yMove)
    j =  stepperY.run();

    if(zMove)
    k =  stepperZ.run();
  }

  return; 
}


void Loop(int xperiod, int yperiod, int zperiod)
{
  
  if(XA0Flag or XmaxAFlag or X0BFlag or XmaxBFlag or Y0Flag or YMAXFlag or Z0Flag or ZMAXFlag)
  {
   //Serial.println("return");
   return;
  }
  GoTo(xperiod, yperiod, zperiod); 

  xperiod = -xperiod; 
  yperiod = -yperiod; 
  zperiod = -zperiod; 
  
}


// Function to calibrate all axis and set MAX and 
// 0 points for all axis individually 
void Calibrate()
{
  
    while(!XA0Flag or !XmaxAFlag or !X0BFlag or !XmaxBFlag or !Y0Flag or !YMAXFlag or !Z0Flag or !ZMAXFlag)
    {

      // XA 0 point calibration 
      if(!XA0Flag)
      {   
      stepperX.setSpeed(-XCalSpeed); 
      stepperX.runSpeed(); 
      }

      // XB 0 point calibration 
      if(!X0BFlag)
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
      if(XA0Flag and !XmaxAFlag)
      {
        stepperX.setSpeed(XCalSpeed); 
        stepperX.runSpeed();
      }

      // XB Max Point Calibration 
      if(X0BFlag and !XmaxBFlag)
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

    XA0Flag = 1; 
    X0BFlag = false; 
    XmaxAFlag = false; 
    XmaxBFlag = false; 
    Y0Flag = false; 
    YMAXFlag = false; 
    Z0Flag = false; 
    ZMAXFlag = false; 

    // set calibration flag 
    calibrated = true; 
   
}


// Function to Check the validity of the interrupts and act accordingly
int checkInts(volatile int flag, int bump, int pos, AccelStepper stepper, bool move)
{
  if(flag == true)
  {
    delay(5); 

    if(digitalRead(bump) == LOW)
    {
    delay(10); 
    Serial.println("Switch has been Pressed"); 
    pos = stepper.currentPosition(); 
    move = false; 
    flag = 0; 
    }
    Serial.println("Flag");
    flag = 0; 
    Serial.println(XA0Flag); 
  }

  return flag; 
}



// main Loop 
void loop() {

   if(Loom.LoRa().receive_blocking(5000)){
    Loom.display_data();

    DynamicJsonDocument doc(400);

    const JsonObject coordinates_json = Loom.internal_json(false);

    deserializeJson(doc, /* Is this right? */ coordinates_json); 

    JsonArray contents = doc["contents"];
        
    X_Location = contents[0]["data"]["MM(x)"]; 
    Y_Location = contents[1]["data"]["MM(y)"]; 
    Z_Location = contents[2]["data"]["MM(z)"]; 
    MaxSpeed = contents[3]["data"]["Velocity"]; 
    Spool_Rad_X = contents[4]["data"]["Radius(x)"]; 
    Spool_Rad_YZ = contents[5]["data"]["Radius(yz)"];
   
    LPrintln(X_Location);
    LPrintln(Y_Location);
    LPrintln(Z_Location);

    // Set Max Speed For all Steppers 
    stepperX.setMaxSpeed(MaxSpeed); 
    stepperY.setMaxSpeed(MaxSpeed/2); 
    stepperZ.setMaxSpeed(MaxSpeed/2); 

    // Convert Number of mm to steps for the GoTo Function
    int xsteps = mmToSteps(X_Location, X_SPR, Spool_Rad_X, X_Micro );
    int ysteps = mmToSteps(Y_Location, YZ_SPR, Spool_Rad_YZ, YZ_Micro );
    int zsteps = mmToSteps(Z_Location, YZ_SPR, Spool_Rad_YZ, YZ_Micro );
 
    GoTo(xsteps, ysteps, zsteps); 

  // Uncomment this line to run the HyperRail in a loop for a predefined period
    Loop(xLoopPeriod, yLoopPeriod, zLoopPeriod); 

    XA0Flag = checkInts(XA0Flag, X0ABump, X0A_pos, stepperX, xMove);

    Serial.println(XA0Flag); 

  // if rail is not calibrated then calibrate it 
//  if(!calibrated)
//    Calibrate();

    Loom.add_data("X_Locatiton", "MM", X_Location);
    Loom.add_data("Y_Locatiton", "MM", Y_Location);
    Loom.add_data("Z_Locatiton", "MM", Z_Location);
    Loom.package();
    Loom.display_data();
    Loom.LoRa().send(9);
    
   }

 }
