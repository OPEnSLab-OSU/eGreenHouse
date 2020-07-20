#include "HyperRail_Driver.h" 

 volatile int Move = 1;

 String JsonStr;   // Define a string object to receive JSON data from Processing GUI

 // Length of X,Y,Z axis in meters
 int Location = 0;

 // velocity you would like the rail to move at
 int Velocity; 

 // variables to hold the spool radius for X, Y, Z axis;
 int Spool_Rad = 32;

 // Variables to select what the rail does
 int Goto = 0; 
 int looP = 0; 
 int Reset = 0; 
 int calibrate = 0; 

 // Variable for looping period in loop mode
 int period = 7000; 

 // Position of the back end of the rail set by the calibration function
 int pos; 

 // Position of the front end of the rail set by the calibration function 
 int max_pos;

 // Make flags for the interrupts
 volatile int N0Flag = 0; 
 volatile int MAXFlag = 0;

 // Calibration Speeds
 int CalSpeed = 200; 

 // Define the max stepper speed in Steps/second
 int MaxSpeed = 2500; 

 // calibration has happened or not
bool calibrated = false; 

  // define all interrupt pins for bump switches
 int N0Bump = 18;
 int MaxBump = 20;

 // construct Motor objects 
   AccelStepper stepper(forward, backward);


/*********************************************************
 * Function: onestep 
 * Description: Function to control a single motor step 
 * Parameters: Direction, Stepper Pin, Direction Pin 
 * Pre-Conditions: Functiion is called to step motor  
 * Post-Conditions: Motor moves a step in the appropriate direction 
 * *******************************************************/
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


/*********************************************************
 * Functions: Forward/Backward Motor 
 * Description: Function to abstract motor stepping for each motor
 *              (All functions below are essentially the same)
 * Parameters: None
 * Pre-Conditions: Functiion is called to step motor  
 * Post-Conditions: Motor moves a step in the appropriate direction 
 * *******************************************************/

void forward() {
  onestep(FORWARD, STEP, DIR);
}

void backward() {
  onestep(BACKWARD, STEP, DIR);
}



/*********************************************************
 * Function: Interrupt Functions  
 * Description: Functions to be triggered when Bump Switches are triggered  
 *              (Functions Below are all the same just for different Bump Switches)
 * Parameters: None
 * Pre-Conditions: Bump Switch is triggered 
 * Post-Conditions: Flag is tripped, motor movement will cease 
 * *******************************************************/

 void ISR()
 {
    if(digitalRead(Y0Bump) == LOW)
   {
    Serial.println("Y0"); 
   N0Flag = 1;
   Move = 0; 
   }
 }

void Max_ISR()
{
  //delay(5); 
   if(digitalRead(YMaxBump) == LOW)
   {
    Serial.println("YMax");
  YMAXFlag = 1; 
  yMove = 0;
   }
}


/*********************************************************
 * Function: mmToSteps 
 * Description: Function to convert milimeters to steps for motors  
 * Parameters: mm, Motor steps per revolution, Motor belt radius, microstepping option
 * Pre-Conditions: Function is called to convert user input to steps 
 * Post-Conditions: metric unit is converted to steps for motors to use
 * *******************************************************/
int mmToSteps(double mm, int steps_per_revolution, double belt_radius, int micro) {

        // Serial.print("mm = ");
        // Serial.println(mm); 

        // Serial.print("steps per rev = "); 
        // Serial.println(steps_per_revolution); 

        // Serial.print("Radius = "); 
        // Serial.println(belt_radius); 
  
        return (int) round((mm*micro)/(2*3.14*belt_radius) * steps_per_revolution);
    }

/*********************************************************
 * Function: GoTo 
 * Description: Function moves stepper motors to an X,Y,Z Coordinate
 * Parameters: x, y, z
 * Pre-Conditions: Function is called to move motors 
 * Post-Conditions: Motors are moved to the correct coordinates
 * *******************************************************/
void GoTo(int x)
{

 // Use MoveTo Function to set desired position 
  stepper.moveTo(x);

  int i = 1; 
  while(i == 1)
  {

    if(Move == 1)
    i =  stepper.run(); 

  }

  return; 
}


/*********************************************************
 * Function: Loop 
 * Description: Function runs motors in a loop based on a period  
 * Parameters: none 
 * Pre-Conditions: Function is called run the motors in an infinite loop  
 * Post-Conditions: motors move in an infinite loop until directed otherwise
 * *******************************************************/
void Loop()
{
  
  // if(X0AFlag or XmaxAFlag or X0BFlag or XmaxBFlag or Y0Flag or YMAXFlag or Z0Flag or ZMAXFlag)
  // {
  //  //Serial.println("return");
  //  return;
  // }

  // Set the location for the motors to go to 
  GoTo(period); 

  //  Reverse the direction 
  period = -period;
  
}


/*********************************************************
 * Function: Calibrate 
 * Description: Function runs motors to their zero position, marks the position then runs 
 *              them to their max position and marks the position. 
 * Parameters: none 
 * Pre-Conditions: Function is called through processing  
 * Post-Conditions: rail is calibrated 
 * *******************************************************/
void Calibrate()
  {
    // Loop while not all of the switches have been pressed
    while((N0Flag == 0) or (MAXFlag == 0))
    {

     // Serial.println("Looping"); 

      Serial.print("N0Flag = "); 
      Serial.println(N0Flag);
      Serial.print("MAXFlag = "); 
      Serial.println(MAXFlag);
      
   //   X0AFlag = checkInts(X0AFlag, X0ABump);      
      //Serial.println(X0AFlag);

/*
      // XA 0 point calibration runs while the 0 switch has not been triggered
      if(X0AFlag == 0)
      {  
        Serial.println("-XA0");
      stepperX.setSpeed(-XCalSpeed); 
      stepperX.runSpeed(); 
      }
      // XB 0 point calibration runs while the 0 switch has not been triggered
      if(X0BFlag == 0)
      {
        Serial.println("-XB0"); 
        stepperX.setSpeed(-XCalSpeed); 
        stepperX.runSpeed(); 
      }
      // If both X axis switches have been triggered then 
      // set the 0 position and allow the axis to move again.  
      if((X0AFlag == 1) and (X0BFlag == 1))
      {
          X0_pos = stepperX.currentPosition() + 50; 
          xAMove = 1; 
          xBMove = 1; 
      }
     // Y 0 point calibration runs while Y0 switch has not been triggered
      if(0 == Y0Flag)
      {
        Serial.println("-Y0"); 
        stepperY.setSpeed(-YZCalSpeed);
        stepperY.runSpeed();
      }
      // When switch is triggered set 0 position and allow the Y axis to move again
      if(Y0Flag == 1)
      {
        Y0_pos = stepperY.currentPosition() + 50; 
        yMove = 1; 
      }
      // Z 0 point calibration runs while the Z0 flag has not been triggered 
      if(0 == Z0Flag)
      {
        Serial.println("-Z0"); 
       stepperZ.setSpeed(-YZCalSpeed); 
       stepperZ.runSpeed(); 
      }
      // When Z0 flag is triggered set 0 position and allow the Z axis to move again
      if(Z0Flag == 1)
      {
        Z0_pos = stepperZ.currentPosition() + 50; 
        zMove = 1; 
      }
     // XA Max Point calibration runs after X0 switches have been triggered 
     // and while Maximum flag has not been triggered
      if((X0AFlag == 1) and (X0BFlag == 1) and (XmaxAFlag == 0))
      {
        Serial.println("XMaxA"); 
        stepperX.setSpeed(XCalSpeed); 
        stepperX.runSpeed();
      }
      // XB Max Point Calibration runs after X0 switches have been triggered 
     // and while Maximum flag has not been triggered
      if((X0BFlag == 1) and (X0AFlag == 1) and (XmaxBFlag == 0))
      {
        Serial.println("XMaxB"); 
        stepperX.setSpeed(XCalSpeed); 
        stepperX.runSpeed();
      }
      // When both X motors reach the maximum switches, set Max position 
      // and allow the x motors to move again
      if((XmaxAFlag == 1) and (XmaxBFlag = 1))
      {
          Xmax_pos = stepperX.currentPosition() - 50; 
          xAMove = 1; 
          xBMove = 1; 
      }
      // Y Max Point Calibration runs while the Y0 switch 
      // has been triggered and before the Ymax switch is triggered
      if((Y0Flag == 1) and (YMAXFlag == 0))
      {
        Serial.println("YMax"); 
        stepperY.setSpeed(YZCalSpeed); 
        stepperY.runSpeed();
      }
      // When Ymax switch is triggered, set max position 
      // and allow the Y motor to move again
      if(YMAXFlag == 1)
      {
        Ymax_pos = stepperY.currentPosition() - 50; 
        yMove = 1; 
      }
      // Z max point calibration runs after Z0 switch has been pressed 
      // and before the Zmax switch is triggered
      if((Z0Flag == 1) and (ZMAXFlag == 0))
      {
        Serial.println("ZMax");
        stepperZ.setSpeed(YZCalSpeed); 
        stepperZ.runSpeed();
      }
      // When Zmax switch is triggered set max position and 
      // allow the Z motor to move again
      if(ZMAXFlag == 1)
      {
        Zmax_pos = stepperZ.currentPosition() - 50; 
        zMove = 1; 
      }
    }
  // make motors move off of the max bump switch
  stepperX.moveTo(Xmax_pos); 
  stepperY.moveTo(Ymax_pos);
  stepperZ.moveTo(Zmax_pos);
  // This is nessecary because the GoTo function will 
  // not run while the flag is triggered
  int i = 1, j = 1, k = 1; 
  while(i == 1 or j == 1 or k == 1)
  {
    i =  stepperX.run(); 
    j =  stepperY.run();
    k =  stepperZ.run();
  }
    // Reset all of the bump switch flags
    X0AFlag = 0; 
    X0BFlag = 0; 
    XmaxAFlag = 0; 
    XmaxBFlag = 0; 
    Y0Flag = 0; 
    YMAXFlag = 0; 
    Z0Flag = 0; 
    ZMAXFlag = 0; 
    // set calibration flag 
    calibrated = true; 
    // Move motors to the 0 position
    GoTo(X0_pos, Y0_pos, Z0_pos); */
   
  } 

}


/*********************************************************
 * Function: checkInts 
 * Description: Function to check the validity of interrupts 
 * Parameters: none
 * Pre-Conditions: Function will check to see if interrupt is valid
 * Post-Conditions: Interrupt flag is set to the correct value
 * *******************************************************/
/*
void checkInts()
{
  if(X0AFlag == 1)
  {
    delay(5); 
    if(digitalRead(X0ABump) == LOW)
      xAMove = 0; 
    else
      X0AFlag == 0;    
  }
}
*/