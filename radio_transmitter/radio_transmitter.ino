#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//define wireless radio pins
#define  CE_PIN  7   // The pins to be used for CE and SN
#define  CSN_PIN 8

//define joystick
#define JOYSTICK_X   A0  // The Joystick potentiometers connected to Arduino Analog inputs
#define JOYSTICK_Y   A1
#define JOYSTICK_SW  2  // The Joystick push-down switch, will be used as a Digital input

RF24 radio(CE_PIN, CSN_PIN);

const byte addresses[][6] = {"1Node", "2Node"}; // These will be the names of the "Pipes"

unsigned long timeNow;  // Used to grab the current time, calculate delays
unsigned long startedWaitingAt;
boolean timeout;       // Timeout? True or False

struct dataStruct {
  unsigned long _micros;  // to save response times
  int xPosition;          // The Joystick position values
  int yPosition;
  bool switchOn;          // The Joystick push-down switch
} myData;                 // This can be accessed in the form:  myData.xPosition  etc.

void setup() {
  pinMode(JOYSTICK_SW, INPUT_PULLUP);  // Pin A2 will be used as a digital input
  
  radio.begin();
  radio.openWritingPipe(addresses[0]);
  radio.openReadingPipe(1, addresses[1]);
  radio.setPALevel(RF24_PA_LOW);
  radio.startListening();
}

void loop() {

  radio.stopListening();
  
  /*********************( Read the Joystick positions )*************************/
  myData.xPosition = analogRead(JOYSTICK_X);
  myData.yPosition = analogRead(JOYSTICK_Y);
  myData.switchOn  = !digitalRead(JOYSTICK_SW);  // Invert the pulldown switch
  myData._micros = micros();  // Send back for timing

  if (!radio.write( &myData, sizeof(myData) )) {	// Send data, checking for error ("!" means NOT)
    Serial.println(F("Transmit failed "));
  }

  // Now, continue listening
  radio.startListening();                                    

  startedWaitingAt = micros();               // timeout period, get the current microseconds
  timeout = false;                           //  variable to indicate if a response was received or not

  while ( ! radio.available() ) {                        
    if (micros() - startedWaitingAt > 200000 ) {           // If waited longer than 200ms, indicate timeout and exit while loop
      timeout = true;
      break;
    }
  }

  if (timeout) {
    Serial.println(F("Response timed out -  no Acknowledge."));
  } else {
    // Grab the response, compare, and send to Serial Monitor
    radio.read( &myData, sizeof(myData) );
    timeNow = micros();

    // Show it
    Serial.print(F("Sent "));
    Serial.print(timeNow);
    Serial.print(F(", Got response "));
    Serial.print(myData._micros);
    Serial.print(F(", Round-trip delay "));
    Serial.print(timeNow - myData._micros);
    Serial.println(F(" microseconds "));
  }
  
  delay(100);
}
