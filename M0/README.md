## eGreenhouse ###
eGreenhouse is a lightweight and low-cost greenhouse sensor package integrated with the OPEnS HyperRail linear motion system. The sensors move through the greenhouse while transmitting a live stream of data to Google Sheets and saving to a microSD card. 

//Must run all .ino files with loom_translator.h and lora.h alongside in the same folder// 

### eGreenhouse has 3 main components: 
  1. The sensor package\
    - Adafruit Feather M0 LoRa + DS3231 RTC + Adalogger Featherwing
    - K30 CO2 sensor\
    - TSL2561 luminosity sensor\
    - SHT31-D temperature + relative humidity sensor\
    
    Future developments underway:
    RFID soil moisture
    Plant stress monitoring via SparkFun Spectro Triad sensor
  
  2. The HyperRail hub (+ optional Ethernet/Wifi transmission)
    - Adafruit Feather M0 LoRa + Ethernet FeatherWing\
    - Optional: make this hub an intermediate node - send data to a third nRF receiver and upload via Ethernet from there. 
  
  3. The graphical user interface (GUI) which runs in the program, Processing. 
  
  We recommend using our custom PCB to to simplify wiring.
 
### The basic sequence:
Upload eGreenhouseSensorsFinal.ino to the sensor package first. Once running, it will wait to receive a command from the HyperRail hub. Next, upload HyperSense_nRF_hub to the HyperRail hub and HyperSenseEthernetNode to the Ethernet hub. Finally, open the Processing GUI and specify the desired sampling locations and intervals. Click 'Travel' and the whole sequence will begin. 

More details + wiring coming soon! 
 
