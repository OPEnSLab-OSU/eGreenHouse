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
String port = "COM5";
String X_Axis_Length = ""; 
String Y_Axis_Length = "";
String Z_Axis_Length = ""; 
String Velocity = ""; 
String SpoolRadX = ""; 
String SpoolRadYZ = ""; 

JSONObject json = new JSONObject();
void setup(){
  
  json.setInt("val", 0);

  size(900, 600);   // Window Size 
  
  myPort = new Serial(this, port, 9600);  // Specify COM Port 
  
  HyperGUI = new ControlP5(this);
  PFont font = createFont("arial", 20); 
  
  
  //*****************X Length Input Box***********
  HyperGUI.addTextfield("X-Axis Position (mm)")
    .setPosition(75, 20)  
    .setSize(200, 40)
    .setFont(font)
    .setFocus(  true)
    .setColor(color(240, 255, 255))
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
    ;
    
     //*****************Z-Axis Length Input Box ***********
  HyperGUI.addTextfield("Z-Axis Position (mm)")
    .setPosition(625, 20)
    .setSize(200, 40)
    .setFont(font)
    .setFocus(true)
    .setColor(color(240, 255, 255))
    ;
    
    //*****************Velocity Input Box ***********
    HyperGUI.addTextfield("Velocity")
      .setPosition(75, 100)
      .setSize(200, 40)
      .setFont(font)
      .setFocus(true)
      .setColor(color(240, 255, 255))
      ;
    
    //*****************X Spool Radius Input Box ***********
    HyperGUI.addTextfield("X-Axis Spool Radius")
      .setPosition(350, 100)
      .setSize(200, 40)
      .setFont(font)
      .setFocus(true)
      .setColor(color(240, 255, 255))
      ; 
      
    //*****************Y,Z Spool Radius Input Box ***********
    HyperGUI.addTextfield("Y/Z-Axis Spool Radius")
      .setPosition(625, 100)
      .setSize(200, 40)
      .setFont(font)
      .setFocus(true)
      .setColor(color(240, 255, 255))
      ; 
    
    HyperGUI.addBang("Submit").setPosition(700, 500).setSize(100, 50);    
    
    textFont(font); 
    
}

void draw()
{
 background(0); 
 
 fill(0, 255, 0); 
 
 
  
}


public void Xaxis(String X_Length)
{
  println("The X-Axis Length has changed to: " + X_Length); 
  X_Axis_Length = X_Length; 
  
}

void Submit()
{
  //Get lengths of all axis and convert to ints
  int X_Axis_Length = Integer.parseInt(HyperGUI.get(Textfield.class, "X-Axis Position (mm)").getText()); 
  int Y_Axis_Length = Integer.parseInt(HyperGUI.get(Textfield.class, "Y-Axis Position (mm)").getText()); 
  int Z_Axis_Length = Integer.parseInt(HyperGUI.get(Textfield.class, "Z-Axis Position (mm)").getText()); 
  
  
  
  // Get Velocity and X,Y,Z Spool Radius
  int Velocity = Integer.parseInt(HyperGUI.get(Textfield.class, "Velocity").getText()); 
  int SpoolRadX = Integer.parseInt(HyperGUI.get(Textfield.class, "X-Axis Spool Radius").getText()); 
  int SpoolRadYZ = Integer.parseInt(HyperGUI.get(Textfield.class, "Y/Z-Axis Spool Radius").getText()); 
  
  json.setInt("X_Location", X_Axis_Length); 
  json.setInt("Y_Location", Y_Axis_Length); 
  json.setInt("Z_Location", Z_Axis_Length); 
  json.setInt("Velocity", Velocity); 
  json.setInt("SpoolRadX", SpoolRadX); 
  json.setInt("SpoolRadYZ", SpoolRadYZ); 
  

  myPort.write(json.toString());
  
}
