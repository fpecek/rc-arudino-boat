#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <SoftwareServo.h>  // Regular Servo library creates timer conflict!

//define wireless
#define  CE_PIN  7   // The pins to be used for CE and SN
#define  CSN_PIN 8

//define servo
#define SERVO_PIN A0   //Pin Numbers for servos and laser/LED

//servi min and max
#define SERVO_MIN_H  30  // Don't go to very end of servo travel
#define SERVO_MAX_H  150 // which may not be all the way from 0 to 180. 
#define SERVO_MIN_V  30  // Don't go to very end of servo travel
#define SERVO_MAX_V  150 // which may not be all the way from 0 to 180

RF24 radio(CE_PIN, CSN_PIN);

SoftwareServo Servo;

const byte addresses[][6] = {"1Node", "2Node"}; // These will be the names of the "Pipes"

int joystickReceived; // Variable to store received Joystick values
int servoPosition;    // variable to store the servo position

struct dataStruct {
  unsigned long _micros;  // to save response times
  int xPosition;          // The Joystick position values
  int yPosition;
  bool switchOn;          // The Joystick push-down switch
} myData;                 // This can be accessed in the form:  myData.xPosition  etc.

void setup() {
  Serial.begin(9600);
  
  Servo.attach(SERVO_PIN);  // attaches the servo to the servo object
    
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.openWritingPipe(addresses[1]);
  radio.openReadingPipe(1, addresses[0]);

  // Start the radio listening for data
  radio.startListening();
}

void loop() {
  if (radio.available()) {
    while (radio.available())   // While there is data ready to be retrieved from the receive pipe
    {
      radio.read( &myData, sizeof(myData) );             // Get the data
    }

    radio.stopListening();                               // First, stop listening so we can transmit
    radio.write( &myData, sizeof(myData) );              // Send the received data back.
    radio.startListening();                              // Now, resume listening so we catch the next packets.

    Serial.print(F("Packet Received - Sent response "));  // Print the received packet data
    Serial.print(myData._micros);
    Serial.print(F("uS X= "));
    Serial.print(myData.xPosition);
    Serial.print(F(" Y= "));
    Serial.print(myData.yPosition);
    if ( myData.switchOn == 1)
    {
      Serial.println(F(" Switch ON"));
    }
    else
    {
      Serial.println(F(" Switch OFF"));
    }
  }
  
  SoftwareServo::refresh();//refreshes servo to keep them updating
  joystickReceived  = myData.xPosition;  // Get the values received

  // scale it to use it with the servo (value between MIN and MAX)
  servoPosition  = map(joystickReceived, 0, 1023, SERVO_MIN_H , SERVO_MAX_H);

  // tell servos to go to position
  Servo.write(servoPosition);
}
