
void sendSerialDebug(String message);
bool sendHttpRequest(String data);
long int statusMessage(String Version);
void initSyncTime();
bool initConnection();
void debugMessage(String message);
void disconnectConnection();


int getPowerUp();         // Excitation power-up
int getSendInterval();    // number of samples collected to transmit
int getLogInterval();      // Set the log interval in minutes 
int getStatusInterval();  //  Status message interval in minutes (default to 1 day)
int getTimeout();         // length of time to wait for cellular connection (milliseconds)
int getFloatDelay();  // Seconds from when float is tripped to set alarm
int getRebootInterval(); // Days between reboots