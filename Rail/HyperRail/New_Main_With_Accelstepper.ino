 // File: HyperRail_Main_New.ino
 // Name: Liam Duncan 
 //        Made for OPEnS Lab 
 // Date: 1/21/2020
 // Description: Code to Drive the HyperRail, Interface with Processing GUI, and Receive data from eGreenhouse

 #include <Loom.h> 
 #include <ArduinoJson.h>
 #include <AccelStepper.h>

 const char* json_config = 
 "{\"general\":{\"device_name\":\"Device\",\"family\":\"Loom\",\"instance_num\":1,\"family_num\":0},\"components\":[{\"name\":\"Loom_DS3231\",\"params\":[11,true]},{\"name\":\"Loom_Interrupt_Manager\",\"params\":[0]},{\"name\":\"Loom_Sleep_Manager\",\"params\":[true,false,1]}]}"
 ;
 
 // Set enabled modules
 LoomFactory<
  Enable::Internet::Disabled,
  Enable::Sensors::Enabled,
  Enable::Radios::Disabled,
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


// Function Gets data from serial port GUI, 
// turns it into a JSON object, and sets int  
// values 
void GetData()
{
  // create JSON object
  DynamicJsonDocument doc(200); 

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
  MaxSpeed = doc["Velocity"]; 
  Spool_Rad_X = doc["SpoolRadX"]; 
  Spool_Rad_YZ = doc["SpoolRadYZ"]; 

  // Set Max Speed For all Steppers 
  stepperX.setMaxSpeed(MaxSpeed); 
  stepperY.setMaxSpeed(MaxSpeed/2); 
  stepperZ.setMaxSpeed(MaxSpeed/2); 


  return; 
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



// main Loop 
void loop() {

  // Read Json Object from Processing
  if(Serial.available()) 
    GetData();   

// Convert Number of mm to steps for the GoTo Function
  int xsteps = mmToSteps(X_Location, X_SPR, Spool_Rad_X, X_Micro );
  int ysteps = mmToSteps(Y_Location, YZ_SPR, Spool_Rad_YZ, YZ_Micro );
  int zsteps = mmToSteps(Z_Location, YZ_SPR, Spool_Rad_YZ, YZ_Micro );
 
  GoTo(xsteps, ysteps, zsteps); 

Serial.print("String = ");
Serial.println(JsonStr); 


  // if rail is not calibrated then calibrate it 
//  if(!calibrated)
//    Calibrate(); 

 }
