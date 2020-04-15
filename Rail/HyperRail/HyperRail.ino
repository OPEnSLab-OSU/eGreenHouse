// File: HyperRail_Main_New.ino
// Name: Liam Duncan 
//        Made for OPEnS Lab 
// Date: 1/21/2020
// Description: Code to Drive the HyperRail, Interface with Processing GUI, and Receive data from eGreenhouse

//#include <Loom.h> 
#include <ArduinoJson.h>
#include <AccelStepper.h>
//#include <Adafruit_MotorShield.h>

// need to think about using this library for the X axis so that it can all be in sync
#include <MultiStepper.h>


//const char* json_config = 
// "{\"general\":{\"device_name\":\"Device\",\"family\":\"Loom\",\"instance_num\":1,\"family_num\":0},\"components\":[{\"name\":\"Loom_DS3231\",\"params\":[11,true]},{\"name\":\"Loom_Interrupt_Manager\",\"params\":[0]},{\"name\":\"Loom_Sleep_Manager\",\"params\":[true,false,1]}]}"
//;
//
//// Set enabled modules
//LoomFactory<
//  Enable::Internet::Disabled,
//  Enable::Sensors::Enabled,
//  Enable::Radios::Disabled,
//  Enable::Actuators::Disabled,
//  Enable::Max::Disabled
//> ModuleFactory{};
//
//LoomManager Loom{ &ModuleFactory };


// Use this space to define the steps per revolution for X, Y and Z motors
#define X_Motor_Steps_Per_Rev  1700 
#define Y_Motor_Steps_Per_Rev  400 
#define Z_Motor_Steps_Per_Rev  400 

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
bool GotData = false; 

// Length of X,Y,Z axis in meters
int X_Location = 0; 
int Y_Location = 0; 
int Z_Location = 0; 

// velocity you would like the rail to move at
int Velocity; 

int Spool_Rad_X = 32; 
int Spool_Rad_YZ = 32; 

// Length of X,Y,Z Axis converted from Meters to steps 
int XA0_pos; 
int XB0_pos; 
int Y0_pos; 
int Z0_pos; 
int X0_pos; 


int XA_pos = 0; 
int XB_pos = 0; 
int Y_pos = 0; 
int Z_pos = 0; 
int X_pos = 0; 

// Length of X,Y,Z Axis converted from Meters to steps 
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
int YZCalSpeed = 500; 

// Max Speed 
int MaxSpeed = 1000; 


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
    delayMicroseconds(5);
// step

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
  XA_pos++; 
  XB_pos++; 
}

void backwardX()
{
  onestep(BACKWARD, STEPXA, DIRXA);
  onestep(BACKWARD, STEPXB, DIRXB);

  XA_pos--;
  XB_pos--; 
}

void backwardXB() {
  onestep(BACKWARD, STEPXB, DIRXB);
}

void forwardY() {
  onestep(FORWARD, STEPY, DIRY);
  Y_pos++;

}

void backwardY() {
  onestep(BACKWARD, STEPY, DIRY);
  Y_pos--; 
}


void forwardZ() {
  onestep(FORWARD, STEPZ, DIRZ);
  Z_pos++; 
}

void backwardZ() {
  onestep(BACKWARD, STEPZ, DIRZ);
  Z_pos--;
}


// construct Motor objects 
  AccelStepper stepperXA(forwardXA, backwardXA);
  AccelStepper stepperXB(forwardXB, backwardXB);
  AccelStepper stepperY(forwardY, backwardY);
  AccelStepper stepperZ(forwardZ, backwardZ); // use functions to step 


MultiStepper XSteppers; 


// put interrupt functions here 

void X0A_ISR()
{
  delay(5); 
  XA0_Count++; 

  if(XA0_Count == 1)
  {
  XA0_pos = XA_pos; 
  XA0Flag = true; 
  Serial.println("XA0 Flag"); 
  Serial.println(XA0_pos);
  }
  else
  XA0_Count = 0; 
}



void Y0_ISR()
{
  delay(5); 
  Y0_Count++; 

  if(Y0_Count == 1)
  {
  Y0_pos = Y_pos; 
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
  delay(5); 
  XB0_Count++; 

  if(XB0_Count == 1)
  {
  XB0_pos = XB_pos; 
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
  delay(5);   // debounce 
  Z0_Count++;   // count to avoid double triggering 

  if(Z0_Count == 1) // if pressed 
  {
  Z0_pos = Z_pos; 
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
  delay(5); 
  XAMAX_Count++; 

  if(XAMAX_Count == 1)
  {
  XAMAX_pos = XA_pos; 
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
  delay(5); 
  YMAX_Count++; 

  if(YMAX_Count == 1)
  {
  YMAX_pos = Y_pos; 
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
  delay(5); 
  XBMAX_Count++; 

  if(XBMAX_Count == 1)
  {
  XBMAX_pos = XB_pos; 
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
  delay(5);   // debounce 
  ZMAX_Count++;   // count to avoid double triggering 

  if(ZMAX_Count == 1) // if pressed 
  {
  ZMAX_pos = Z_pos; 
  //stepperZ.currentPosition();  // set current position 
  ZMAXFlag = true;                        // set flag 
  Serial.println("ZMAX Flag");            // print things for testing 
  Serial.println(ZMAX_pos);
  }
  else                                  // reset counter when interrupt is triggered on release of switch 
  ZMAX_Count = 0;  
}



void setup() {

  Serial.begin(9600); 

  Serial.print("Arduino is Ready!"); 

  
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
  stepperXA.setMaxSpeed(MaxSpeed); 
  stepperXB.setMaxSpeed(MaxSpeed); 
  stepperY.setMaxSpeed(MaxSpeed); 
  stepperZ.setMaxSpeed(MaxSpeed); 

  stepperXA.setAcceleration(100); 

  // MultiStepper 
  XSteppers.addStepper(stepperXA); 
  XSteppers.addStepper(stepperXB); 
  

  
  // initialize all interrupts for Bump Switches 

//  // X0A Interrupt 
//  Loom.InterruptManager().register_ISR(X0ABump, X0A_ISR, FALLING, ISR_Type::IMMEDIATE);
//
//  // X0B Interrupt 
//  Loom.InterruptManager().register_ISR(X0BBump, X0B_ISR, FALLING, ISR_Type::IMMEDIATE);
//
//  // XMaxA Interrupt 
// Loom.InterruptManager().register_ISR(XMaxABump, XMaxA_ISR, FALLING, ISR_Type::IMMEDIATE);
//
//  // XMaxB Interrupt 
//  Loom.InterruptManager().register_ISR(XMaxBBump, XMaxB_ISR, FALLING, ISR_Type::IMMEDIATE);
//
//  // Y0 Interrupt 
//  Loom.InterruptManager().register_ISR(Y0Bump, Y0_ISR, FALLING, ISR_Type::IMMEDIATE);
//
//  // YMax Interrupt 
//  Loom.InterruptManager().register_ISR(YMaxBump, YMax_ISR, FALLING, ISR_Type::IMMEDIATE);
//
//  // Z0 Interrupt 
//  Loom.InterruptManager().register_ISR(Z0Bump, Z0_ISR, FALLING, ISR_Type::IMMEDIATE);
//
//  // ZMax Interrupt 
//  Loom.InterruptManager().register_ISR(ZMaxBump, ZMax_ISR, FALLING, ISR_Type::IMMEDIATE);

}


int mmToSteps(double mm, int steps_per_revolution, double belt_radius) {

        Serial.print("mm = ");
        Serial.println(mm); 

        Serial.print("steps per rev = "); 
        Serial.println(steps_per_revolution); 

        Serial.print("Radius = "); 
        Serial.println(belt_radius); 
  
        return (int) round(mm/(2*3.14*belt_radius) * steps_per_revolution);
    }

//int mmToSteps(int Radius, int mm)
//{
//  
//  
//  float degree = mm/Radius; 
//
//  float steps = 360/degree; 
//
//  return steps; 
//}




// Function Gets data from serial port GUI, 
// turns it into a JSON object, and sets int  
// values 
void GetData()
{
  // create JSON object
  DynamicJsonDocument doc(200); 

  doc = Loom.internal_json(); 

  Serial.print(doc); 

  // receive Json String From Processing   
  JsonStr = Serial.readString(); 
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
  X_Location = doc["X_Location"]; 
  Y_Location = doc["Y_Location"]; 
  Z_Location = doc["Z_Location"]; 
  Velocity = doc["Velocity"]; 
  Spool_Rad_X = doc["SpoolRadX"]; 
  Spool_Rad_YZ = doc["SpoolRadYZ"]; 
  
   Serial.print("X_Location = ");
   Serial.println(X_Location); 

  // Set Bool to true so that this function does not run again 
  GotData = true; 
  return; 
}




void GoTo(int x, int y, int z)
{

  // take the average of the two X motors 
  int X_pos = (XA_pos + XB_pos)/2; 
  

  // set the distance in steps that each motor must go 
  int x_steps = x - X_pos; 
  int y_steps = y - Y_pos; 
  int z_steps = z - Z_pos; 

  Serial.print("x_steps = "); 
  Serial.println(x_steps); 
  

 
//  // loop to step the apropriate number of steps and log positions 
//  for(int i = 0; i<abs(x_steps); i++)
//  {
//    if(x_steps > 0)
//      forwardX();
//    else
//      backwardX(); 
//    delay(2);
//  }
//
//    // loop to go apropriate number of steps and log positions 
//    for(int j = 0; j<abs(y_steps); j++)
//    {
//      if(y_steps > 0)
//      forwardY();
//      else 
//      backwardY(); 
//
//      delay(1); 
//    }
//
//    // loop to move Z motor and log position 
//      for (int k = 0; k<abs(z_steps); k++)
//      {
//        if(z_steps > 0)
//        forwardZ(); 
//        else
//        backwardZ(); 
//        delay(2);    
//        Z_pos++;  
//        //stepperZ.currentPosition(); 
//      }


// while loop should be able to move all motors simultaniously 

int i = 0, j = 0, k = 0; 
while(i<abs(x_steps) and j<abs(y_steps) and k<abs(z_steps))
{
  if(x_steps > 0)
  {
    forwardX();
    i++; 
    delay(1);
  }


  if(x_steps < 0)
  {
    backwardX(); 
    i++; 
    delay(1);

  }

  if(y_steps > 0)
  {
    forwardY(); 
    j++;
    delay(1);

  }

  if(y_steps < 0)
  {
    backwardY(); 
    j++; 
    delay(1);

  }

  if(z_steps > 0)
  {
    forwardZ(); 
    k++; 
    delay(1);

  }

  if(z_steps < 0)
  {
    backwardZ(); 
    k++;    
    delay(1);

  }
  
}

  
  return; 
}




// Function to calibrate all axis and set MAX and 
// 0 points for all axis individually 
void Calibrate()
{
  // flags to make sure that you are not accidentally setting positions
  bool xaMaxing = false; 
  bool xbMaxing = false; 
  bool yMaxing = false; 
  bool zMaxing = false; 
  
    while(!XA0Flag or !XAMAXFlag or !XB0Flag or !XBMAXFlag or !Y0Flag or !YMAXFlag or !Z0Flag or !ZMAXFlag)
    {



//      if(!XA0Flag or !XB0Flag)
//      {
//        XSteppers.setSpeed(-XCalSpeed); 
//        XSteppers.runSpeed(); 
//      }

      // XA 0 point calibration 
      if(!XA0Flag)
      {
        backwardXA(); 
        delay(1);
        XA_pos--; 
//        XSteppers.setSpeed(-XCalSpeed); 
//        XSteppers.runSpeed(); 
      }
     // else if(!xaMaxing)
//      {
//        // logs 0 point for motor XA in steps 
//        XA0_pos = stepperXA.currentPosition(); 
//      }

      // XB 0 point calibration 
      if(!XB0Flag)
      {
          backwardXB(); 
          delay(1); 
          XB_pos--; 
//        stepperXB.setSpeed(-XCalSpeed); 
//        stepperXB.runSpeed(); 
      }
//      else if(!xbMaxing)
//      {
//        // logs the 0 point for motor XB in steps 
//        XB0_pos = stepperXB.currentPosition(); 
//      }

      // Y 0 point calibration 
      if(!Y0Flag)
      {
        backwardY(); 
        delay(1); 
        Y_pos--; 
       }
//      else if(!yMaxing)
//      {
//        // logs the 0 point for the Y axis in steps 
//        Y0_pos = stepperY.currentPosition(); 
//      }

      // Z 0 point calibration 
      if(!Z0Flag)
      {
        backwardZ(); 
        delay(1);
        Z_pos--; 
//        stepperZ.setSpeed(-YZCalSpeed); 
//        stepperZ.runSpeed(); 
      }
//      else if(!zMaxing)
//      {
//        // log Z0 point in steps 
//        Z0_pos = stepperZ.currentPosition(); 
//      }


      // XA Max Point calibration 
      if(XA0Flag and !XAMAXFlag)
      {
        // go find max X point 
        //xaMaxing = true; 
//        stepperXA.setSpeed(XCalSpeed); 
//        stepperXA.runSpeed();
          forwardXA(); 
          delay(1);
          XA_pos++; 
      }
//      else if(xaMaxing) 
//      {
//        // log X max point 
//        XAMAX_pos = stepperXA.currentPosition(); 
//      }


      // XB Max Point Calibration 
      if(XB0Flag and !XBMAXFlag)
      {
        // go find max X point 
//        xbMaxing = true; 
//        stepperXB.setSpeed(XCalSpeed); 
//        stepperXB.runSpeed();
          forwardXB(); 
          delay(1); 
          XB_pos++; 
      }
//      else if(xbMaxing)
//      {
//        // log X max point in steps 
//        XBMAX_pos = stepperXB.currentPosition(); 
//      }

      // Y Max Point Calibration 
      if(Y0Flag and !YMAXFlag)
      {
        // go find max Y point 
//        yMaxing = true; 
//        stepperY.setSpeed(YZCalSpeed); 
//        stepperY.runSpeed();

         forwardY();
         delay(1);  
         Y_pos++; 
      }
//      else if(yMaxing) 
//      {
//        // log Y max Position
//        YMAX_pos = stepperY.currentPosition(); 
//      }


      // Z max point calibration
      if(Z0Flag and !ZMAXFlag)
      {
        // go find max Z point 
//        zMaxing = true; 
//        stepperZ.setSpeed(YZCalSpeed); 
//        stepperXA.runSpeed();

          forwardZ(); 
          delay(1); 
          Z_pos++; 
      }
//      else if(zMaxing)
//      {
//        // log max Z point 
//        ZMAX_pos = stepperZ.currentPosition(); 
//      }
    }

    // reset all flags
    xaMaxing = false; 
    xbMaxing = false; 
    yMaxing = false; 
    zMaxing = false; 


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



// main Loop 
void loop() {

  // Read Json Object from Processing
  if(Loom.LoRa().receive_blocking(5000))
  //if(Serial.available())
  {
//  JsonStr = Serial.readString(); 
//  Serial.println(JsonStr); 
    GetData();   
  }

// Convert Number of mm to steps for the GoTo Function
int xsteps =  mmToSteps(X_Location, X_Motor_Steps_Per_Rev ,Spool_Rad_X );

int ysteps =  mmToSteps(Y_Location, Y_Motor_Steps_Per_Rev, Spool_Rad_YZ );
 
    GoTo(xsteps, ysteps , 400); 


  // if rail is not calibrated then calibrate it 
//  if(!calibrated)
//    Calibrate(); 


}
