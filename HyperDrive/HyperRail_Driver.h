#ifndef HYPERRAIL_DRIVER_H
#define HYPERRAIL_DRIVER_H

#include <ArduinoJson.h>
#include <AccelStepper.h>

 // define the steps per revolution for X,Y and Z motors 
 #define X_SPR 1700
 #define YZ_SPR 400 


 // Use these variables to define your microstep values for the motors 
 // When there is nothing connected to the big easy driver pins the driver defaults to 1/16 step 
 #define YZ_Micro 1 // 1/16 step 
 #define X_Micro 1  // full step 


  // define all interrupt pins for bump switches
 extern int X0ABump;
 extern int X0BBump;

 extern int XMaxABump;
 extern int XMaxBBump;

 extern int Y0Bump ;
 extern int YMaxBump ;

 extern int Z0Bump ;
 extern int ZMaxBump ;

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

 extern volatile int xAMove; 
 extern volatile int xBMove; 
 extern volatile int yMove; 
 extern volatile int zMove; 

 extern String JsonStr;   // Define a string object to receive JSON data from Processing GUI

 // Length of X,Y,Z axis in meters
 extern int X_Location; 
 extern int Y_Location; 
 extern int Z_Location; 

 // velocity you would like the rail to move at
 extern int Velocity; 

 // variables to hold the spool radius for X, Y, Z axis;
 extern int Spool_Rad_X; 
 extern int Spool_Rad_YZ; 

 // Variables to select what the rail does
 extern int Goto; 
 extern int looP; 
 extern int Reset; 
 extern int calibrate; 

 // Variable for looping period in loop mode
 extern int periodX;
 extern int periodY; 
 extern int periodZ; 

 // Position of the back end of the rail set by the calibration function
 extern int X0A_pos; 
 extern int X0B_pos; 
 extern int Y0_pos; 
 extern int Z0_pos; 
 extern int X0_pos; 

 // Position of the front end of the rail set by the calibration function
 extern int XmaxA_pos; 
 extern int XmaxB_pos; 
 extern int Ymax_pos; 
 extern int Zmax_pos; 
 extern int Xmax_pos;

 // Make flags for the interrupts 
 extern volatile int X0AFlag; 
 extern volatile int XmaxAFlag; 
 extern volatile int X0BFlag; 
 extern volatile int XmaxBFlag; 
 extern volatile int Y0Flag; 
 extern volatile int YMAXFlag; 
 extern volatile int Z0Flag; 
 extern volatile int ZMAXFlag; 

 // Calibration Speeds
 extern int XCalSpeed; 
 extern int YZCalSpeed; 

 // Define the max stepper speed in Steps/second
 extern int MaxSpeed; 

 // calibration has happened or not
extern bool calibrated; 



void onestep(int dir, int stepPin, int dirPin); 

void forwardXA();

void backwardXA();

void forwardXB();

void forwardX();

void backwardX();

void backwardXB();

void forwardY(); 

void backwardY();

void forwardZ();

void backwardZ();

void X0A_ISR();

void Y0_ISR();

void X0B_ISR();

void Z0_ISR();

void XMaxA_ISR();

void YMax_ISR();

void XMaxB_ISR();

void ZMax_ISR();

int mmToSteps(double mm, int steps_per_revolution, double belt_radius, int micro);

void GetData();

void GoTo(int x, int y, int z);

void Loop();

void Calibrate();

void checkInts();



 // construct Motor objects 
   extern AccelStepper stepperX;
   extern AccelStepper stepperY;
   extern AccelStepper stepperZ; 





#endif 