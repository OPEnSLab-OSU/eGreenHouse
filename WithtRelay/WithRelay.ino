// This program will send data to the HyperDrive Program with these sensors
#include <Loom.h>

// CO2 Sensor: K30 (done)
#include "wiring_private.h"

// Luminosity Sensor(Light Sensor): TSL2591 (done)
// Temperature & Relative Humidity Sensor: SHT31-D (done)
// Particle Sensor(Dust & Pollen): SDS-011 (don't work on it: just for the fan)(done)

// This one will set and do an alarm for every 60 second by RTC(real time clock)

//Include Configuration
const char* json_config =
#include "config.h"
;

// Set enabled modules
LoomFactory<
  Enable::Internet::Disabled,
  Enable::Sensors::Enabled,
  Enable::Radios::Enabled,
  Enable::Actuators::Disabled,
  Enable::Max::Disabled
> ModuleFactory{};

LoomManager Loom{ &ModuleFactory };

const int theSetPin = A4;
const int theUnsetPin = A5;
const int AlarmPin = 10;    // Pin DS3231 INT pin is physically connected to


//Create Serial SERCOM for K30 Sensor: RX pin 12, TX pin 11

Uart Serial2 = Uart(&sercom1, 12, 11, SERCOM_RX_PAD_3, UART_TX_PAD_0);

void setup() 
{
  Loom.power_up();
  pinMode(theSetPin, OUTPUT);  // Set pin, output
  pinMode(theUnsetPin, OUTPUT);  // Unset pin, output
  relayOn(); // Power on attached sensor for setup!
  
  Loom.begin_LED();
  digitalWrite(LED_BUILTIN, HIGH);
  
  while(!Serial) {}

  Serial2.begin(9600);
  Loom.begin_serial(true);
  Loom.parse_config(json_config);
  Loom.print_config();

  Loom.InterruptManager().register_ISR(AlarmPin, wakeISR, LOW, ISR_Type::IMMEDIATE);

  digitalWrite(LED_BUILTIN, LOW);

  //   Assign pins 10 & 11 SERCOM functionality
  pinPeripheral(11, PIO_SERCOM);
  pinPeripheral(12, PIO_SERCOM);
  
  Loom.K30().set_serial(&Serial2);

  LPrintln("\n **eGreenHouse Sensor Collector Ready** ");                              //Indicating the user that setup function is complete

}


void loop() 
{
  Loom.power_up();
  LPrintln("Powering up sensor");
  relayOn(); // Power up sensor via Relay

  // Flash pattern to visually indicate processor has turned on
  digitalWrite(LED_BUILTIN, HIGH);
  delay(250);
  digitalWrite(LED_BUILTIN, LOW);
  delay(250);

  Loom.measure(); // Sample attached sensors
  Loom.package(); // Format data for display and SD
  Loom.display_data(); // display printed JSON formatted data on serial monitor

  Loom.LoRa().send(6);

  Loom.InterruptManager().RTC_alarm_duration(0,0,0,60); // Wakes up the sensors every 60 seconds
  Loom.InterruptManager().reconnect_interrupt(AlarmPin); // Important, make interrupt pin sensitive to alarm signal again

  Loom.SDCARD().log("Ken.csv");

  LPrintln("Powering down sensor");
  relayOff(); // Power down sensor via Relay
  
  LPrintln("Going to sleep");

  Loom.SleepManager().sleep(); // rest in low power sleep mode

  Loom.power_down();
  
  LPrintln("Awake");
}

// subroutine to set A4 and A5 pins to turn relay on
void relayOn()
{
  // Set relay signal pins to known state, both low
  digitalWrite(theUnsetPin, LOW);  // turn the LED on (LOW is the voltage level)
  digitalWrite(theSetPin, LOW);   // turn the LED on (LOW is the voltage level)

  // To turn relay on, set theSetPin High for 20ms, then set low again
  digitalWrite(theSetPin, HIGH);
  delay(20);

}

// subroutine to set A4 and A5 pins to turn relay off
void relayOff()
{
  // Set relay signal pins to known state, both low
  digitalWrite(theUnsetPin, LOW);  // turn the LED on (LOW is the voltage level)
  digitalWrite(theSetPin, LOW);   // turn the LED on (LOW is the voltage level)

  // To turn relay on, set theSetPin High for 20ms, then set low again
  digitalWrite(theUnsetPin, HIGH);
  delay(20);
  digitalWrite(theUnsetPin, LOW); // Turn off the UnsetPin LED to save battery
}

// Subroutine that is executed when the RTC timer signal triggers the
// wake up interrupt service routine
void wakeISR() {

  detachInterrupt(AlarmPin);
  LPrintln("Alarm went off");

}

void SERCOM1_Handler()
{
  Serial2.IrqHandler();
}
