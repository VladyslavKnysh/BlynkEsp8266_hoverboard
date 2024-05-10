
// ########################## DEFINES ##########################
#define HOVER_SERIAL_BAUD   115200      // [-] Baud rate for HoverSerial (used to communicate with the hoverboard)
#define SERIAL_BAUD         115200      // [-] Baud rate for built-in Serial (used for the Serial Monitor)
#define START_FRAME         0xABCD     	// [-] Start frme definition for reliable serial communication
#define TIME_SEND           100           // [ms] Sending time interval
#define SPEED_MAX_TEST      1000         // [-] Maximum speed for testing
#define SPEED_STEP          15          // [-] Speed step
//#define STEER_STEP          20          // [-] Steer step
// #define DEBUG_RX                        // [-] Debug received data. Prints all bytes to serial (comment-out to disable)
#define RXD D7   //RXD
#define TXD D8  //TXD

#define BLYNK_TEMPLATE_ID "TMPL4N56yhbMD"
#define BLYNK_TEMPLATE_NAME "telega"
#define BLYNK_AUTH_TOKEN "_e_gBraqUXjD7Mgp3H90A5IyxQpK5D6G"

//#define BLYNK_PRINT Serial

// ########################## INCLUDES ##########################
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>


#include "SoftwareSerial.h"

//Crtl variables
unsigned long iTimeSend = 0;
int iStep = SPEED_STEP;
// int jStep = STEER_STEP;
int speed = 0;
int steer = 0;
int speedSetp;
int steerSetp;
SoftwareSerial HoverSerial(RXD,TXD);        // RX, TX

// char ssid[] = "lab-2-17";
// char pass[] = "98351541";
char ssid[] = "lab-2-17";
char pass[] = "98351541";

// Global variables
uint8_t idx = 0;                        // Index for new data pointer
uint16_t bufStartFrame;                 // Buffer Start Frame
byte *p;                                // Pointer declaration for the new received data
byte incomingByte;
byte incomingBytePrev;

typedef struct{
   uint16_t start;
   int16_t  steer;
   int16_t  speed;
   uint16_t checksum;
} SerialCommand;
SerialCommand Command;

typedef struct{
   uint16_t start;
   int16_t  cmd1;
   int16_t  cmd2;
   int16_t  speedR_meas;
   int16_t  speedL_meas;
   int16_t  batVoltage;
   int16_t  boardTemp;
   uint16_t cmdLed;
   uint16_t checksum;
} SerialFeedback;
SerialFeedback Feedback;
SerialFeedback NewFeedback;

//int LED_BUILTIN = 16;

// ########################## SETUP ##########################
void setup() 
{
  Serial.begin(SERIAL_BAUD);
  Serial.println("Hoverboard Serial v1.0");

  HoverSerial.begin(HOVER_SERIAL_BAUD);
  pinMode(LED_BUILTIN, OUTPUT);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
}

// ########################## LOOP ##########################

void loop(void)
{ 
  Blynk.run();

  unsigned long timeNow = millis();

  // Check for new received data
  Receive();

  // Send commands
  if (iTimeSend > timeNow) return;
  iTimeSend = timeNow + TIME_SEND;

  // Calculate test command signal
  if (speedSetp>speed){
    iStep = SPEED_STEP;
  }else if (speedSetp<speed){
    iStep = -SPEED_STEP;
  }
  // if (steerSetp>steer){
  //   jStep = STEER_STEP;
  // }else if (steerSetp<steer){
  //   jStep = -STEER_STEP;
  // }

  if(speed != speedSetp){
    speed += iStep;
  }
  steer = steerSetp;
  // if(steer != steerSetp){
  //   steer += jStep;
  // }
  
  Send(steer, speed);
  blynkFeedback();

  // Blink the LED
  digitalWrite(LED_BUILTIN, (timeNow%2000)<1000);
}

void smoothStop(){
  speedSetp = 0;
  steerSetp = 0;  
  
  steer = 0;
  // while(speed!=0 && steer!=0){
  while(speed!=0){
    if(abs(speed - 0)>200){
      speed = speed - speed/1.3;
    }else if(speed != 0){
      speed = 0;
      }
    
    // if(abs(steer - 0)>200){
    //   steer = steer - steer/1.3;
    // }else if(steer != 0){
    //   steer = 0;
    //   }

    Send(speed,steer);
    blynkFeedback();
  }
}

// ########################## END ##########################
