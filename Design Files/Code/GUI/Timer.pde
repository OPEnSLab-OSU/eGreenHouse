/*******************************************************************************
** This is the Timer.pde file.                                                **
**                                                                            **
** This is the sub program for setting up the timer for the eGreenhouse.pde.  **
**                                                                            ** 
** Please do not modify the code here.                                        **
**                                                                            **
** Author: Kenneth Kang                                                       **
**                                                                            **
********************************************************************************/

class Timer{
  float Time;
  Timer(float set){
    Time = set;
  }
  float getTime(){
    return(Time);
  }
  void setTime(float set){
    Time = set;
  }
  void countDown(){
    Time -= 1/frameRate;
  }
}
