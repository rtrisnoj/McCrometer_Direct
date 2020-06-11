#include "Particle.h"
#include "HttpClient.h"

int powerUp = 5;              
int sendInterval = 24;        
int  logInterval = 5;       
int statusInterval = 1440;   
int timeout = 10000;   
bool debug = false;
float floatDelay = 20;     
int rebootInterval = 7;

void sendSerialDebug(String message);
int getPowerUp() { return powerUp;}             
int getSendInterval() {return sendInterval;}       
int getLogInterval() {return logInterval;}        
int getStatusInterval() {return statusInterval;}  
int getTimeout() {return timeout;}  
int getFloatDelay() {return floatDelay;}
int getRebootInterval() {return rebootInterval;}

http_header_t headers[] = {
    { "Content-Type", "application/json" },
    { "Authorization" , "Token yWNjZXNzM" },
    { "Accept" , "*/*"},
    { NULL, NULL } 
};
HttpClient http;
http_request_t request;
http_response_t response;

String hostname = "ws.uscubed.com";
int hostport = 80;
String hostpath = "/ParticleDirect.aspx";

bool initConnection()
{
  if (Cellular.ready())
    return true;
  bool retVal = false;
  Cellular.on();
  Cellular.connect();
  waitFor(Cellular.ready,timeout);
  Particle.connect();
  waitFor(Particle.connected,timeout);
  if (Cellular.ready())
    retVal = true;
  delay(1000);
  return retVal;
}

bool sendHttpRequest(String data){
  if (initConnection()){
    Particle.syncTime();
    String id = spark_deviceID();

    request.hostname = hostname;
    request.port = hostport;
    request.path = hostpath;
    request.body = id + ";"  + data;
    sendSerialDebug(request.body);
    http.get(request, response, headers);
    sendSerialDebug("After sendHTTPRequest() http.get()");

    if (response.body == "OK")
      return true;
  }
  return false;
}

void disconnectConnection()
{
  Particle.disconnect();
  waitUntil(Particle.disconnected);
  sendSerialDebug("disconnectConnection(): before Cellular.off()");
  Cellular.off();
  sendSerialDebug("disconnectConnection(): after Cellular.off()"); 
}
void debugMessage(String message){
 if(debug)
 {
    sendHttpRequest(message);
 }
}
bool setParameter(String param, String value)
{ if (param == "si")
  {
    sendInterval = value.toInt();
  }
  else if (param == "li")
  {
      logInterval = value.toInt();
  }
  else if (param == "sm")
  {
      statusInterval = value.toInt();
  }
  else if (param == "pu")
  {
      powerUp = value.toInt();
  }
  else if (param == "fd")
  {
      floatDelay = value.toInt();
  }
  else if (param == "ri")
  {
      rebootInterval = value.toInt();
  }
  else if (param == "to")
  {
      timeout = value.toInt();
      if (timeout < 10000)  // Ensure that an invalid timeout parameter does not disable radio... minimum timeout is 10 seconds
        timeout = 10000;
  }
  else if (param == "db")
  {
      if(value == "1")
        debug = true;
      else
        debug = false;
  }
  else
  {
      sendSerialDebug("Unknown parameter- " + param + ":" + value);
  }
  
  return true;
}

void parseString(String strVars)
{
  sendSerialDebug("Parsing Response:" + strVars);
  strVars = strVars.replace("\"","");
  String parameter = ""; 
  String value = "";
  String inChar = "";
  int i = 0;
  bool readingParam = true;

  while (inChar != ".")
  {
  inChar = strVars.charAt(i++);
  if (inChar != ":" and inChar != ",")
    {
    if (readingParam)
      parameter += inChar;
    else
      value += inChar;    
    }
  else
    {
      if (inChar == ":")
      {
        readingParam = false;
      }
      else
      {
        if (setParameter(parameter, value) == false)
          return;
        parameter = "";
        value = ""; 
        readingParam = true;
      }
    }
  }
  setParameter(parameter,value.replace(".",""));
}
/****  validate response to status by sending back new parameters  ****/
void responseMessage()
{
  FuelGauge fuel;
  if (initConnection())
  {
    String message =  String(Time.now()) + 
    ",Reply" +
    ",li:" + (String)logInterval + 
    ",si:" + (String)sendInterval + 
    ",sm:" + (String)statusInterval + 
    ",to:" + (String)timeout + 
    ",pu:" + (String)powerUp + 
    ",fd:" + (String)floatDelay +
    ",ri:" + (String)rebootInterval +
    ",Bat:" + (String)fuel.getVCell();
    sendSerialDebug(message);
    sendHttpRequest(message);
    sendSerialDebug("After send response");

  }
}
/****  Every version of code should contain a daily status message  ****/
long int statusMessage(String Version)
{
  FuelGauge fuel;
  if (initConnection())
  {
    Particle.syncTime();
    delay(2000);
    CellularSignal sig = Cellular.RSSI();
    int rssi = sig.rssi;
    String id = spark_deviceID();

    String message =  id + ";" + String(Time.now()) + 
       ",Status" +
       ",li:" + (String)logInterval + 
       ",si:" + (String)sendInterval + 
       ",sm:" + (String)statusInterval + 
       ",to:" + (String)timeout +
       ",pu:" + (String)powerUp + 
       ",ver:" + Version +
       ",RSSI:" + (String)rssi+
       ",Bat:" + (String)fuel.getVCell(); 

    request.hostname = hostname;
    request.port = hostport;
    request.path = hostpath;
    request.body = message;
    sendSerialDebug(message);
    http.get(request, response, headers);
    String resp = response.body;
    parseString(resp);
    responseMessage();
  }
  long int lastStatusMessage = Time.now() + 60;
  return lastStatusMessage;
}


void initSyncTime()
{
  initConnection();
  Particle.syncTime();
}

