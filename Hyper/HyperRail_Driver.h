#ifndef HYPERRAIL_DRIVER_H
#define HYPERRAIL_DRIVER_H

#include <ArduinoJson.h>
#include <AccelStepper.h>

 // define the steps per revolution for X,Y and Z motors 
 #define SPR 400 


 // Use these variables to define your microstep values for the motors 
 // When there is nothing connected to the big easy driver pins the driver defaults to 1/16 step 
 #define Micro 1 // 1/16 step


  // define all interrupt pins for bump switches
 extern int N0Bump ;
 extern int MaxBump ;

 // Define Motor Pins for Y 
 #define STEP 9
 #define DIR 16

 // foreward and backward definitions
 #define FORWARD 0
 #define BACKWARD 1

 extern volatile int Move; 
 extern String JsonStr;   // Define a string object to receive JSON data from Processing GUI

 // Length of X,Y,Z axis in meters
 extern int Location;

 // velocity you would like the rail to move at
 extern int Velocity; 

 // variables to hold the spool radius for X, Y, Z axis;
 extern int Spool_Rad; 

 // Variables to select what the rail does
 extern int Goto; 
 extern int looP; 
 extern int Reset; 
 extern int calibrate; 

 // Variable for looping period in loop mode
 extern int period; 

 // Position of the back end of the rail set by the calibration function 
 extern int pos;

 // Position of the front end of the rail set by the calibration function 
 extern int max_pos;

 // Make flags for the interrupts 
 extern volatile int N0Flag; 
 extern volatile int MAXFlag;

 // Calibration Speeds 
 extern int CalSpeed; 

 // Define the max stepper speed in Steps/second
 extern int MaxSpeed; 

 // calibration has happened or not
extern bool calibrated; 



void onestep(int dir, int stepPin, int dirPin);

void forward(); 

void backward();

void ISR();

void Max_ISR();

int mmToSteps(double mm, int steps_per_revolution, double belt_radius, int micro);

void GetData();

void GoTo(int x);

void Loop();

void Calibrate();

//void checkInts();



 // construct Motor objects 
   extern AccelStepper stepper;

#endif 