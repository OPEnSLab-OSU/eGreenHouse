/*****************************************************************************************************
** This is the GUI.pde file.                                                                        **
**                                                                                                  **
** This will open a window box that has total of 4 input boxes, texts, a timer for 6 minutes,       **
** and 4 buttons.                                                                                   **
**                                                                                                  ** 
** The box input will only take integers.                                                           **
** Each box are what values to change.                                                              **
**                                                                                                  **
** The texts will inform what to do or what to know.                                                **
**                                                                                                  **
** The timer is for the K30 Sensor warm up time. Please the text or eGreenhouse_Sensor_Package.ino  **
**                                                                                                  **
** Last the button are self-explanatory what they do.                                               **
**                                                                                                  **
** Author: Kenneth Kang                                                                             **
**                                                                                                  **
******************************************************************************************************/


/****************************************
*********Importing Libraries*************
****************************************/
import controlP5.*;//GUI
import processing.serial.*;//Serial
import cc.arduino.*;

/****************************************
*********Declaring instances*************
****************************************/
ControlP5 HyperGUI; // GUI instance
Serial myPort;//The serial port
Arduino arduino;

/****************************************
*****Declare variable for functions******
****************************************/
String port = "COM11";


String GoTo = "0"; 
int Axis_Length;
int Velocity; 
int SpoolRad; 

String Reset = "0";

String Calibrate = "0";

String Loop = "0";
int period;

String K30_Note = "If you just turn on the Sensors, wait for 6 min to warm up for K30 Sensor!";
String K30_Timer = "This is a 6 minutes timer for the warm up.";
String Message_Meaning = "If it says fail, please check the console log from Processing.";
String K30_Timer_Note = "If you already did the 6 mintues warm up, you can ignore the timer.";
int clicked = 0;

JSONObject json = new JSONObject();
Timer startTimer;
void setup(){
  
  
  
  json.setInt("val", 0);

  size(800, 400);   // Window Size 
  
  myPort = new Serial(this, port, 9600);  // Specify COM Port 
  
  HyperGUI = new ControlP5(this);
  PFont font = createFont("Calibri", 20); 
  
  
  //*****************Length Input Box***********
  HyperGUI.addTextfield("Position (mm)")
    .setPosition(75, 20)
    .setSize(200, 40)
    .setFont(font)
    .setFocus(  true)
    .setColor(color(240, 255, 255))
    .setText("0")
    ;
  
    //*****************Velocity Input Box ***********
    HyperGUI.addTextfield("Velocity")
      .setPosition(350, 20)
      .setSize(200, 40)
      .setFont(font)
      .setFocus(true)
      .setColor(color(240, 255, 255))
      .setText("2500")
      ;
    
    //*****************Spool Radius Input Box ***********
    HyperGUI.addTextfield("Axis Spool Radius")
      .setPosition(350, 100)
      .setSize(200, 40)
      .setFont(font)
      .setFocus(true)
      .setColor(color(240, 255, 255))
      .setText("32")
      ; 
      
    //***************** Period Input Box ***********
    HyperGUI.addTextfield("Period")
      .setPosition(75, 100)
      .setSize(200, 40)
      .setFont(font)
      .setFocus(true)
      .setColor(color(240, 255, 255))
      .setText("0")
      ;     

    
    HyperGUI.addBang("Submit").setPosition(600, 20).setSize(50, 25).getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER);    

    HyperGUI.addBang("Calibrate").setPosition(600, 50).setSize(50,25).getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER); 
    
    HyperGUI.addBang("Reset").setPosition(600, 80).setSize(50,25).getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER); 

    HyperGUI.addBang("Loop").setPosition(600, 110).setSize(50,25).getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER);
    
    textFont(font); 
    
    startTimer = new Timer(360);
    
}

void draw()
{
  int teller = clicked;
  printConsole();
  switch(teller){
    case 0:
      timer();
      textMessage();
      text(startTimer.getTime(), 70, 350);
      text(" seconds left", 125, 350);
      break;
    default:
      printConsole();
      textMessage();
      break;
  }
}


public void textMessage(){


   textFont(createFont("Calibri", 15));
   text("Console Log", 600, 200);
   text(K30_Note, 75, 250);
   text(Message_Meaning, 75, 200);
   text(K30_Timer, 75, 300);
   text(K30_Timer_Note, 75, 325);
  
}

public void printConsole(){
  while (myPort.available() > 0){
     String result = myPort.readString();
     if(result != null){
        background(0);
        fill(0,255,0);
        println(result);
        textFont(createFont("Calibri", 15));
        textMessage();
        text(result, 600, 225);
     }
  }
}
  

public void timer(){
  background(0);
  fill(0,255,0);
  if(startTimer.getTime() > 0){
     startTimer.countDown();
   }else{
     startTimer.setTime(0);
   }
   
}


void Submit()
{
  
  //Get length of axis and convert to ints
  Axis_Length = Integer.parseInt(HyperGUI.get(Textfield.class, "Position (mm)").getText());
  
  // Get Velocity and Spool Radius
  Velocity = Integer.parseInt(HyperGUI.get(Textfield.class, "Velocity").getText()); 
  SpoolRad = Integer.parseInt(HyperGUI.get(Textfield.class, "Axis Spool Radius").getText());

  // Get the Period Loops
  period = Integer.parseInt(HyperGUI.get(Textfield.class, "Period").getText());
  // Add Values to JSON
  json.setInt("GoTo", 1); 
  json.setInt("Location", Axis_Length); 
  json.setInt("Velocity", Velocity); 
  json.setInt("SpoolRad", SpoolRad);

  json.setInt("Loop", 0); 
  json.setInt("Reset", 0); 
  json.setInt("Calibrate", 0); 

  json.setInt("period", period);

  clicked = 1;
  myPort.write(json.toString()); 
}

void Calibrate()
{
  //Get length of axis and convert to ints
  Axis_Length = Integer.parseInt(HyperGUI.get(Textfield.class, "Position (mm)").getText());
  
  
  // Get Velocity and Spool Radius
  Velocity = Integer.parseInt(HyperGUI.get(Textfield.class, "Velocity").getText()); 
  SpoolRad = Integer.parseInt(HyperGUI.get(Textfield.class, "Axis Spool Radius").getText());

  // Get the Period Loops
  period = Integer.parseInt(HyperGUI.get(Textfield.class, "Period").getText());
  // Add Values to JSON
  json.setInt("GoTo", 1); 
  json.setInt("Location", Axis_Length); 
  json.setInt("Velocity", Velocity); 
  json.setInt("SpoolRad", SpoolRad);

  json.setInt("Loop", 0); 
  json.setInt("Reset", 0); 
  json.setInt("Calibrate", 1); 

  json.setInt("period", period);

  clicked = 1;
  myPort.write(json.toString());
}


void Reset()
{
  //Get length of axis and convert to ints
  Axis_Length = Integer.parseInt(HyperGUI.get(Textfield.class, "Position (mm)").getText());
  
  
  // Get Velocity and Spool Radius
  Velocity = Integer.parseInt(HyperGUI.get(Textfield.class, "Velocity").getText()); 
  SpoolRad = Integer.parseInt(HyperGUI.get(Textfield.class, "Axis Spool Radius").getText());

  // Get the Period Loops
  period = Integer.parseInt(HyperGUI.get(Textfield.class, "Period").getText());
  // Add Values to JSON
  json.setInt("GoTo", 1); 
  json.setInt("Location", Axis_Length); 
  json.setInt("Velocity", Velocity); 
  json.setInt("SpoolRad", SpoolRad);

  json.setInt("Loop", 0); 
  json.setInt("Reset", 1); 
  json.setInt("Calibrate", 0); 

  json.setInt("period", period);

  clicked = 1;
  myPort.write(json.toString());
}

void Loop()
{
  //Get length of axis and convert to ints
  Axis_Length = Integer.parseInt(HyperGUI.get(Textfield.class, "Position (mm)").getText());
  
  
  // Get Velocity and Spool Radius
  Velocity = Integer.parseInt(HyperGUI.get(Textfield.class, "Velocity").getText()); 
  SpoolRad = Integer.parseInt(HyperGUI.get(Textfield.class, "Axis Spool Radius").getText());

  // Get the Period Loops
  period = Integer.parseInt(HyperGUI.get(Textfield.class, "Period").getText());
  // Add Values to JSON
  json.setInt("GoTo", 1); 
  json.setInt("Location", Axis_Length); 
  json.setInt("Velocity", Velocity); 
  json.setInt("SpoolRad", SpoolRad); 

  json.setInt("Loop", 1); 
  json.setInt("Reset", 0); 
  json.setInt("Calibrate", 0); 

  json.setInt("period", period);

  clicked = 1;
  myPort.write(json.toString());
}
