// error display and logging routines
// main purpose is to simulate a delay with an error flashing light
//


// print configuration on startup for log entry and debug
void dumpVars(void) {
  sprintf(logbuf, "Hostname: %s", configHostname);
  logWrite(logbuf);
  sprintf(logbuf, "Website: %s", configWebsite);
  logWrite(logbuf);
  sprintf(logbuf, "WiFi: %s", configWifi);
  logWrite(logbuf);
  sprintf(logbuf, "eMail: %s", configEmail);
  logWrite(logbuf);
  sprintf(logbuf, "Verify Email: %s", configVerifyEmail);
  logWrite(logbuf);
  sprintf(logbuf, "Test Email: %s", configTestEmail);
  logWrite(logbuf);
  sprintf(logbuf, "Alert Email: %s", configAlertEmail);
  logWrite(logbuf);
  sprintf(logbuf, "Return Email: %s", configReturnEmail);
  logWrite(logbuf);
  sprintf(logbuf, "Alert Timer: %d", workingAlertTimer);
  logWrite(logbuf);
  sprintf(logbuf, "Power Timer: %d", workingPowerTimer);
  logWrite(logbuf);
  sprintf(logbuf, "Log Timer: %d", workingLogTimer);
  logWrite(logbuf);
  sprintf(logbuf, "Log Mode: %d", workingLogMode);
  logWrite(logbuf);
  sprintf(logbuf, "Morning Timer Start: %d", workingMorningTimerStart);
  logWrite(logbuf);
  sprintf(logbuf, "Morning Timer End: %d", workingMorningTimerEnd);
  logWrite(logbuf);
  sprintf(logbuf, "Test Days: %s", workingTestDays);
  logWrite(logbuf);

 // would usefully be expanded to also include email content
}
// the following are startup errors and are not expected to exit until a reset
// one flash means waiting for wifi
// simple routine to flash LED until first wifi connact --------------------
void waitWiFi(void) {
  int oldstate = errorLED;
  digitalWrite(outputEmail, HIGH);
  delay(100);
  digitalWrite(outputEmail, LOW);
  delay(100);
  digitalWrite(outputEmail, HIGH);
  delay(500);
  if (!oldstate) {
    digitalWrite(outputEmail, HIGH);
  }
}
// simple routine to flash LED when SD card missing or unusable -----------------
// two flashes mean bad sd card
void badSD(void) {
  int oldstate = errorLED;
  digitalWrite(outputEmail, HIGH);
  delay(100);
  digitalWrite(outputEmail, LOW);
  delay(100);
  digitalWrite(outputEmail, HIGH);
  delay(100);
  digitalWrite(outputEmail, LOW);
  delay(100);
  digitalWrite(outputEmail, HIGH);
  delay(500);
  if (!oldstate) {
    digitalWrite(outputEmail, HIGH);
  }
}
// simple routine to flash LED when SD card config incorrect
// three flashes means bad configuration
void badConfig(void) {
  int oldstate = errorLED;
  digitalWrite(outputEmail, HIGH);
  delay(100);
  digitalWrite(outputEmail, LOW);
  delay(100);
  digitalWrite(outputEmail, HIGH);
  delay(100);
  digitalWrite(outputEmail, LOW);
  delay(100);
  digitalWrite(outputEmail, HIGH);
  delay(100);
  digitalWrite(outputEmail, LOW);
  delay(100);
  digitalWrite(outputEmail, HIGH);
  delay(500);
  if (!oldstate) {
    digitalWrite(outputEmail, HIGH);
  }
}

// simple routine to flash LED if time cannot be set --------------------------
// four flashes means that time cannot be set...indicates that internet not accessible
void noTime(void) {
  //  may also be no internet
  int oldstate = errorLED;
  digitalWrite(outputEmail, HIGH);
  delay(100);
  digitalWrite(outputEmail, LOW);
  delay(100);
  digitalWrite(outputEmail, HIGH);
  delay(100);
  digitalWrite(outputEmail, LOW);
  delay(100);
  digitalWrite(outputEmail, HIGH);
  delay(100);
  digitalWrite(outputEmail, LOW);
  delay(100);
  digitalWrite(outputEmail, HIGH);
  delay(100);
  digitalWrite(outputEmail, LOW);
  delay(100);
  digitalWrite(outputEmail, HIGH);
  delay(500);

  if (!oldstate) {
    digitalWrite(outputEmail, HIGH);
  }
}
