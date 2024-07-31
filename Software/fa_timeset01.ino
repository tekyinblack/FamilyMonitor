// time handling functions --------------------------------------

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0000;
const int daylightOffset_sec = 3600;

// legacy print time function for testing ----------------------------------
void printLocalTime() {
  struct tm timeinfo;
  char buffer[80];
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    timeSet = 0;
    return;
  } else timeSet = 1;
  // Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");

  strftime(buffer, 80, "%A, %B %d %Y %H:%M:%S", &timeinfo);
  Serial.println(buffer);
}

// initial setup of time for RTC -----------------------------------------------
int time_setup(void) {

  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;

  // if wifi down then no possibility of getting a new time fix
  if (WiFi.status() != WL_CONNECTED) {
    return false;
  }

  // attempt time fix, if it fails then return a failure status
  if (!getLocalTime(&timeinfo)) {
    logWrite("Failed to obtain time");
    return false;
    // else if it has worked, then set then indicate time set
  } else {
    timeSet = 1;
    logWrite("Time Set");
    return true;
  }
}
