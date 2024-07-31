/* Routines to maintain and report wifi

*/
// check and display wifi --------------------------------------
void displayWiFi() {
  //  Is Wifi connected
  //         YES
  //         Turn on LED
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(outputWiFi, LOW);   // turn on led
    // update wifi status to true
    statusWiFi = 1;
  } else {     // wifi not connected
    digitalWrite(outputWiFi, HIGH);    // turn off led
    statusWiFi = 0;      // update wifi statu sto false
    stableWiFi = 0;      // update stable wifi status to false
    
  }

  // if wifi status has changed, log message

  if (oldStatusWiFi == 0 && statusWiFi == 1) {
    logWrite("WiFi connected to SSID " + String(ssid) + " IP address: " + WiFi.localIP().toString());
    time_setup();    // get time to verify connectivity
    time(&workingTime);    // update working reference time
    wifiTimer = workingTime + wifiStableTime;   // set stable wifi timer
  }
  if (oldStatusWiFi == 1 && statusWiFi == 0) {
    reconnectInterval = RECONNECT_BASE;    // initialise wifi reconnect timer
    logWrite("WiFi disconnected from SSID " + String(ssid));
  }

  oldStatusWiFi = statusWiFi;   // save wifi status for use if status changes
}


// test and reconnect wifi -------------------------------------------
void reconnectWiFi() {
  currentReconnect = millis();
  // if WiFi is down, try reconnecting on a timed interval
  if ((WiFi.status() != WL_CONNECTED) && (currentReconnect - previousReconnect >= reconnectInterval)) {

    logWrite("Reconnecting to SSID " + String(ssid));
    WiFi.disconnect();
    WiFi.begin(ssid, password);


    // set the next reconnect interval to increase until max interval reached
    // this is to stop continula reconnections when wifi is switched off
    previousReconnect = currentReconnect;
    reconnectInterval = reconnectInterval + RECONNECT_INC;
    if (reconnectInterval >= RECONNECT_MAX) {
      reconnectInterval = RECONNECT_MAX;
    }
  }
}
