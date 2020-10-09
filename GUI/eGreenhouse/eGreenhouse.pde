/*****************************************************************************************************
** This is the eGreenhouse.pde file.                                                                **
**                                                                                                  **
** This will open a window box that has total of 9 input boxes, texts, a timer for 6 minutes,       **
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
** Author: Kenneth Kang and Liam Duncan                                                             **
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
int X_Axis_Length; 
int Y_Axis_Length;
int Z_Axis_Length;
int Velocity; 
int SpoolRadX; 
int SpoolRadYZ; 

String Reset = "0";

String Calibrate = "0";

String Loop = "0";
int periodX;
int periodY;
int periodZ; 

String K30_Note = "If you just turn on the Sensors, wait for 6 min to warm up for K30 Sensor!";
String K30_Timer = "This is a 6 minutes timer for the warm up.";
String Message_Meaning = "If it says fail, please check the console log from Processing.";
String K30_Timer_Note = "If you already did the 6 mintues warm up, you can ignore the timer.";
int clicked = 0;

JSONObject json = new JSONObject();
Timer startTimer;
void setup(){
  
  json.setInt("val", 0);

  size(900, 600);   // Window Size 
  
  myPort = new Serial(this, port, 9600);  // Specify COM Port 
  
  HyperGUI = new ControlP5(this);
  PFont font = createFont("Georgia", 20); 
  
  
  //*****************X Length Input Box***********
  HyperGUI.addTextfield("X-Axis Position (mm)")
    .setPosition(75, 20)
    .setSize(200, 40)
    .setFont(font)
    .setFocus(  true)
    .setColor(color(240, 255, 255))
    .setText("0")
    ;
  
 //*****************Y length Input Box ***********
  //This is the input text box for the spool Radius/Diameter
  HyperGUI.addTextfield("Y-Axis Position (mm)")
    //.setValue(10)//This will be the default 10mm
    .setPosition(350, 20)
    .setSize(200, 40)
    .setFont(font)
    .setFocus(true)
    .setColor(color(240, 255, 255))
    .setText("0")
    ;
    
     //*****************Z-Axis Length Input Box ***********
  HyperGUI.addTextfield("Z-Axis Position (mm)")
    .setPosition(625, 20)
    .setSize(200, 40)
    .setFont(font)
    .setFocus(true)
    .setColor(color(240, 255, 255))
    .setText("0")
    ;
    
    //*****************Velocity Input Box ***********
    HyperGUI.addTextfield("Velocity")
      .setPosition(75, 100)
      .setSize(200, 40)
      .setFont(font)
      .setFocus(true)
      .setColor(color(240, 255, 255))
      .setText("2500")
      ;
    
    //*****************X Spool Radius Input Box ***********
    HyperGUI.addTextfield("X-Axis Spool Radius")
      .setPosition(350, 100)
      .setSize(200, 40)
      .setFont(font)
      .setFocus(true)
      .setColor(color(240, 255, 255))
      .setText("32")
      ; 
      
    //*****************Y,Z Spool Radius Input Box ***********
    HyperGUI.addTextfield("Y/Z-Axis Spool Radius")
      .setPosition(625, 100)
      .setSize(200, 40)
      .setFont(font)
      .setFocus(true)
      .setColor(color(240, 255, 255))
      .setText("32")
      ; 

    //***************** X-Period Input Box ***********
    HyperGUI.addTextfield("X-Period")
      .setPosition(75, 180)
      .setSize(200, 40)
      .setFont(font)
      .setFocus(true)
      .setColor(color(240, 255, 255))
      .setText("0")
      ;     
      
    //*****************Y-Period Input Box ***********
    HyperGUI.addTextfield("Y-Period")
      .setPosition(350, 180)
      .setSize(200, 40)
      .setFont(font)
      .setFocus(true)
      .setColor(color(240, 255, 255))
      .setText("0")
      ;   
      
    //***************** Z-Period Input Box ***********
    HyperGUI.addTextfield("Z-Period")
      .setPosition(625, 180)
      .setSize(200, 40)
      .setFont(font)
      .setFocus(true)
      .setColor(color(240, 255, 255))
      .setText("0")
      ; 
    
    HyperGUI.addBang("Submit").setPosition(500, 500).setSize(100, 50).getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER);    

    HyperGUI.addBang("Calibrate").setPosition(350, 500).setSize(100,50).getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER); 
    
    HyperGUI.addBang("Reset").setPosition(200, 500).setSize(100,50).getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER); 

    HyperGUI.addBang("Loop").setPosition(50, 500).setSize(100,50).getCaptionLabel().align(ControlP5.CENTER, ControlP5.CENTER);
    
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
      text(startTimer.getTime(), 70, 450);
      text(" seconds left", 125, 450);
      break;
    default:
      printConsole();
      textMessage();
      break;
  }
}


public void textMessage(){


   textFont(createFont("Georgia", 15));
   text("Console Log", 625, 275);
   text(K30_Note, 75, 350);
   text(Message_Meaning, 75, 300);
   text(K30_Timer, 75, 400);
   text(K30_Timer_Note, 75, 425);
  
}

public void printConsole(){
  while (myPort.available() > 0){
     String result = myPort.readString();
     if(result != null){
        background(0);
        fill(0,255,0);
        println(result);
        textFont(createFont("Georgia", 15));
        textMessage();
        text(result, 625, 300);
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

  //Get lengths of all axis and convert to ints
  X_Axis_Length = Integer.parseInt(HyperGUI.get(Textfield.class, "X-Axis Position (mm)").getText());
  Y_Axis_Length = Integer.parseInt(HyperGUI.get(Textfield.class, "Y-Axis Position (mm)").getText()); 
  Z_Axis_Length = Integer.parseInt(HyperGUI.get(Textfield.class, "Z-Axis Position (mm)").getText()); 
  
  
  // Get Velocity and X,Y,Z Spool Radius
  Velocity = Integer.parseInt(HyperGUI.get(Textfield.class, "Velocity").getText()); 
  SpoolRadX = Integer.parseInt(HyperGUI.get(Textfield.class, "X-Axis Spool Radius").getText()); 
  SpoolRadYZ = Integer.parseInt(HyperGUI.get(Textfield.class, "Y/Z-Axis Spool Radius").getText()); 

  // Get the Period of all the Axis Loops
  periodX = Integer.parseInt(HyperGUI.get(Textfield.class, "X-Period").getText()); 
  periodY = Integer.parseInt(HyperGUI.get(Textfield.class, "Y-Period").getText()); 
  periodZ = Integer.parseInt(HyperGUI.get(Textfield.class, "Z-Period").getText()); 
  
  // Add Values to JSON
  json.setInt("GoTo", 1); 
  json.setInt("X_Location", X_Axis_Length); 
  json.setInt("Y_Location", Y_Axis_Length); 
  json.setInt("Z_Location", Z_Axis_Length); 
  json.setInt("Velocity", Velocity); 
  json.setInt("SpoolRadX", SpoolRadX); 
  json.setInt("SpoolRadYZ", SpoolRadYZ); 

  json.setInt("Loop", 0); 
  json.setInt("Reset", 0); 
  json.setInt("Calibrate", 0); 

  json.setInt("periodX", periodX);
  json.setInt("periodY", periodY);
  json.setInt("periodZ", periodZ);

  clicked = 1;
  myPort.write(json.toString()); 
}

void Calibrate()
{
    //Get lengths of all axis and convert to ints
  int X_Axis_Length = Integer.parseInt(HyperGUI.get(Textfield.class, "X-Axis Position (mm)").getText()); 
  int Y_Axis_Length = Integer.parseInt(HyperGUI.get(Textfield.class, "Y-Axis Position (mm)").getText()); 
  int Z_Axis_Length = Integer.parseInt(HyperGUI.get(Textfield.class, "Z-Axis Position (mm)").getText()); 
  
  
  // Get Velocity and X,Y,Z Spool Radius
  int Velocity = Integer.parseInt(HyperGUI.get(Textfield.class, "Velocity").getText()); 
  int SpoolRadX = Integer.parseInt(HyperGUI.get(Textfield.class, "X-Axis Spool Radius").getText()); 
  int SpoolRadYZ = Integer.parseInt(HyperGUI.get(Textfield.class, "Y/Z-Axis Spool Radius").getText()); 

  // Get the Period of all the Axis Loops
  int periodX = Integer.parseInt(HyperGUI.get(Textfield.class, "X-Period").getText()); 
  int periodY = Integer.parseInt(HyperGUI.get(Textfield.class, "Y-Period").getText()); 
  int periodZ = Integer.parseInt(HyperGUI.get(Textfield.class, "Z-Period").getText()); 
  
  // Add Values to JSON
  json.setInt("GoTo", 0); 
  json.setInt("X_Location", X_Axis_Length); 
  json.setInt("Y_Location", Y_Axis_Length); 
  json.setInt("Z_Location", Z_Axis_Length); 
  json.setInt("Velocity", Velocity); 
  json.setInt("SpoolRadX", SpoolRadX); 
  json.setInt("SpoolRadYZ", SpoolRadYZ); 

  json.setInt("Loop", 0); 
  json.setInt("Reset", 0); 
  json.setInt("Calibrate", 1); 

  json.setInt("periodX", periodX);
  json.setInt("periodY", periodY);
  json.setInt("periodZ", periodZ);

  clicked = 1;
  myPort.write(json.toString());
}


void Reset()
{
    //Get lengths of all axis and convert to ints
  int X_Axis_Length = Integer.parseInt(HyperGUI.get(Textfield.class, "X-Axis Position (mm)").getText()); 
  int Y_Axis_Length = Integer.parseInt(HyperGUI.get(Textfield.class, "Y-Axis Position (mm)").getText()); 
  int Z_Axis_Length = Integer.parseInt(HyperGUI.get(Textfield.class, "Z-Axis Position (mm)").getText()); 
  
  
  // Get Velocity and X,Y,Z Spool Radius
  int Velocity = Integer.parseInt(HyperGUI.get(Textfield.class, "Velocity").getText()); 
  int SpoolRadX = Integer.parseInt(HyperGUI.get(Textfield.class, "X-Axis Spool Radius").getText()); 
  int SpoolRadYZ = Integer.parseInt(HyperGUI.get(Textfield.class, "Y/Z-Axis Spool Radius").getText()); 

  // Get the Period of all the Axis Loops
  int periodX = Integer.parseInt(HyperGUI.get(Textfield.class, "X-Period").getText()); 
  int periodY = Integer.parseInt(HyperGUI.get(Textfield.class, "Y-Period").getText()); 
  int periodZ = Integer.parseInt(HyperGUI.get(Textfield.class, "Z-Period").getText()); 
  
  // Add Values to JSON
  json.setInt("GoTo", 0); 
  json.setInt("X_Location", X_Axis_Length); 
  json.setInt("Y_Location", Y_Axis_Length); 
  json.setInt("Z_Location", Z_Axis_Length); 
  json.setInt("Velocity", Velocity); 
  json.setInt("SpoolRadX", SpoolRadX); 
  json.setInt("SpoolRadYZ", SpoolRadYZ); 

  json.setInt("Loop", 0); 
  json.setInt("Reset", 1); 
  json.setInt("Calibrate", 0); 

  json.setInt("periodX", periodX);
  json.setInt("periodY", periodY);
  json.setInt("periodZ", periodZ);

  clicked = 1;
  myPort.write(json.toString());
}

void Loop()
{
    //Get lengths of all axis and convert to ints
  int X_Axis_Length = Integer.parseInt(HyperGUI.get(Textfield.class, "X-Axis Position (mm)").getText()); 
  int Y_Axis_Length = Integer.parseInt(HyperGUI.get(Textfield.class, "Y-Axis Position (mm)").getText()); 
  int Z_Axis_Length = Integer.parseInt(HyperGUI.get(Textfield.class, "Z-Axis Position (mm)").getText()); 
  
  
  // Get Velocity and X,Y,Z Spool Radius
  int Velocity = Integer.parseInt(HyperGUI.get(Textfield.class, "Velocity").getText()); 
  int SpoolRadX = Integer.parseInt(HyperGUI.get(Textfield.class, "X-Axis Spool Radius").getText()); 
  int SpoolRadYZ = Integer.parseInt(HyperGUI.get(Textfield.class, "Y/Z-Axis Spool Radius").getText()); 

  // Get the Period of all the Axis Loops
  int periodX = Integer.parseInt(HyperGUI.get(Textfield.class, "X-Period").getText()); 
  int periodY = Integer.parseInt(HyperGUI.get(Textfield.class, "Y-Period").getText()); 
  int periodZ = Integer.parseInt(HyperGUI.get(Textfield.class, "Z-Period").getText()); 
  
  // Add Values to JSON
  json.setInt("GoTo", 0); 
  json.setInt("X_Location", X_Axis_Length); 
  json.setInt("Y_Location", Y_Axis_Length); 
  json.setInt("Z_Location", Z_Axis_Length); 
  json.setInt("Velocity", Velocity); 
  json.setInt("SpoolRadX", SpoolRadX); 
  json.setInt("SpoolRadYZ", SpoolRadYZ); 

  json.setInt("Loop", 1); 
  json.setInt("Reset", 0); 
  json.setInt("Calibrate", 0); 

  json.setInt("periodX", periodX);
  json.setInt("periodY", periodY);
  json.setInt("periodZ", periodZ);

  clicked = 1;
  myPort.write(json.toString());
}
