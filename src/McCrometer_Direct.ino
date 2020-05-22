/*  Changelog
1.3.0- Separated US3lib from main code
1.3.1- Added reboot routine and Battery check
1.3.2- Added Ryan's code to read RS485
1.3.3- Added Serial debug code
*/

#include "US3lib.h"

String Version = "1.1.3";
const bool serialDebug = true;

int led1 = D0; // Instead of writing D0 over and over again, we'll write led1
// You'll need to wire an LED to this one to see it blink.

int led2 = D7; // Instead of writing D7 over and over again, we'll write led2
// This one is the little blue LED on your board. On the Photon it is next to D7, and on the Core it is next to the USB jack.

const int buttonPin = D4;

long int lastStatusMessage = 0;
long int prevTime;
long int currentTime;
long int sleepTime;


int tripCount = 0;  
int samplesLogged = 0;
String payload;
String totalPayload;
bool booting = true;
int sendAttempts = 1;   // counter to indicate number of send attempt failures 


//The setup function is called once at startup of the sketch

PRODUCT_ID(10618);
PRODUCT_VERSION(1);
SYSTEM_MODE (MANUAL);

void setup() {

  
  //delay(10000);

  pinMode(buttonPin, INPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);

  initSyncTime();
  prevTime = Time.now();       
  delay(1000);
  sendSerialDebug("-------------Program boot------------------------------------");    
  sendHttpRequest("Starting");
  
  //*************************  Test  Block ********************************
  //digitalWrite(pinRelay,HIGH);
  //delay(3000);
  //digitalWrite(pinRelay,LOW);
  //**********************   End Test Block ******************************

  lastStatusMessage = statusMessage(Version);
}

void loop() {
  SleepResult result = System.sleepResult();
  currentTime = Time.now(); 
  delay(1000);
  sendSerialDebug("*************Begin loop()**********************************");


  currentTime = Time.now();

   // *******  Check if wakeup pin has tripped ******
  if (!result.wokenUpByRtc() && !booting) {
    tripCount++;
    sendSerialDebug("Trip Count:");
    sendSerialDebug(String(tripCount));
    
  } 
  
  boolean sendingStatus = false;
  booting = false;


  // ******* Check if status message is due  *****
  if (lastStatusMessage + (60 * getStatusInterval()) < Time.now() )
  {
    lastStatusMessage = statusMessage(Version);
    sendingStatus = true;
  }

  // *********   Take sample *************
  if (currentTime - prevTime >= (getLogInterval() * 60 )) 
  {

    payload += "," + (String)tripCount;
    samplesLogged++;
    prevTime = currentTime;
    
    if (((samplesLogged >= (getSendInterval() * sendAttempts)) && !sendingStatus))
    {
      if (initConnection())
      {
        totalPayload = String(Time.now()-(60*getLogInterval()*(samplesLogged-1)));
        totalPayload += ",1043,";
        totalPayload += String(getLogInterval());
        totalPayload += payload;
        if (sendHttpRequest(totalPayload))   
        {
          payload = "";
          samplesLogged = 0;
          
        }
      }
      else
      {
        sendAttempts++;
      }
    }    
    tripCount = 0;
  }



  // ******** Reboot if due *************
  if ((long int)millis() > (getRebootInterval() * 1440 * 60 * 1000))
  {
    disconnectConnection();
    System.reset();  
  }

  debugMessage("Good Night!");
  sendSerialDebug("before disconnectConnection()");
  disconnectConnection();
  sendSerialDebug("after disconnectConnection()");
  delay(1000);
  System.sleep(buttonPin, RISING , getLogInterval() * 60 );

}


void sendSerialDebug(String message)
{
  if (serialDebug)
  {
  message.replace(",","-");
  String sparkID = spark_deviceID() + ",";
  String time = Time.format(Time.now(), "%Y-%m-%d %H:%M:%S,");
  Serial.println(time + String(millis()) + "," + sparkID + message);
  }
}
