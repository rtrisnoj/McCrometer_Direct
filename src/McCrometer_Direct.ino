/*  Changelog
1.1.3- Initial code by Ryan
1.2.1- Added code to monitor repeating pressure readings
1.2.1- Reinstated boot message for pressue
*/

#include "US3lib.h"

String Version = "m1.2.1";
const bool serialDebug = true;

const int buttonPin = D4;
const int excitePin = A0;
const int analogPin = A5;

long int lastStatusMessage = 0;
long int prevTime;
long int currentTime;
long int sleepTime;


int tripCount = 0;  
int samplesLogged = 0;
String payload;
String payloadAnalog;
String totalPayload;
String totalPayloadAnalog;
bool booting = true;
int sendAttempts = 1;   // counter to indicate number of send attempt failures 


//The setup function is called once at startup of the sketch

PRODUCT_ID(10618);
PRODUCT_VERSION(1);
SYSTEM_MODE (MANUAL);

void setup() {

  Serial.begin(19200);
  //delay(10000);

  pinMode(buttonPin, INPUT);
  pinMode(excitePin, OUTPUT);
  pinMode(analogPin, INPUT);


  initSyncTime();
  prevTime = Time.now();       
  delay(1000);
  sendSerialDebug("-------------Program boot------------------------------------");    
  String an = (String)getAnalogInput();
  sendSerialDebug(an);

  sendHttpRequest("Starting");
  
  lastStatusMessage = statusMessage(Version);
  sendBootMessage();
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
    payloadAnalog += "," + (String)getAnalogInput(); 
    ++samplesLogged; //RT
    prevTime = currentTime;
    
    if (((samplesLogged >= (getSendInterval() * sendAttempts)) && !sendingStatus))
    {
      if (initConnection())
      {
        totalPayload = String(Time.now()-(60*getLogInterval()*(samplesLogged)));
        totalPayloadAnalog = String(Time.now()-(60*getLogInterval()*(samplesLogged)));        
        totalPayload += ",1043,";               
        totalPayloadAnalog += ",1024,";        // Data type for repeating pressure readings
        totalPayload += String(getLogInterval());
        totalPayloadAnalog += String(getLogInterval());        
        totalPayload += payload;
        totalPayloadAnalog += payloadAnalog;        
        if (sendHttpRequest(totalPayload) && sendHttpRequest(totalPayloadAnalog) )   
        {
          payload = "";
          payloadAnalog = "";
          samplesLogged = 0;
          sendAttempts = 1; //RT
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
  System.sleep(buttonPin,FALLING , getLogInterval() * 60 ); //RT

}

int getAnalogInput()
{
  int retVal;
  digitalWrite(excitePin,HIGH);
  delay(1000);
  retVal = analogRead(analogPin);
  digitalWrite(excitePin,LOW);
  
  return retVal;

}

void sendBootMessage()
{ 
  if (initConnection())
  {
    Particle.syncTime();
    String payload = "" ;
    payload = String(Time.now());
    payload += ",1024,";
    payload += String(getLogInterval());
    payload += "," + (String)getAnalogInput();
    sendHttpRequest(payload);
  }
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
