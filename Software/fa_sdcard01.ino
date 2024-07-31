/*
  Library of SDcard routines with modified elements for application use
  Test routine at the end
*/

#define WIFI 1
#define SMTP 2
#define CONFIG 3
#define EMAIL 4
#define STATUS 5

// save alert time to SD card on a regular basis for power off restarts --------==
void saveStatusToSD(void) {

  File file = SD.open("/status.txt", FILE_WRITE);
  if (!file) {
    logWrite("Failed to open Status for writing");
    return;
  } else if (!file.println(workingTime)) {
    logWrite("Status Write failed workingTime");
  } else if (!file.println(workingAlertTime)) {
    logWrite("Status Write failed workingAlertTime");
  } else if (!file.println(workingTestTime)) {
    logWrite("Status Write failed workingTestTime");
  } else if (!file.println(alertState)) {
    logWrite("Status Write failed alertState");
  } else if (!file.println(workingLastSeen)) {
    logWrite("Status Write failed workingLastSeen");
  }

  file.close();
  logWrite("Status written");
}

// logwrite writes the message to a file and to the serial log --------------------
void logWrite(String logOut) {



  // write the message to the serial log come what may

  // verify that the log is available
  // if not try to open it for appending

  // if so,then append to current log

  // if timeset=0 then don't append time

  // put time first
  time_t logTime;
  struct tm *timeinfo;
  char buffer[40];

  time(&logTime);
  timeinfo = localtime(&logTime);

  strftime(buffer, 80, "%A, %B %d %Y %H:%M:%S", timeinfo);

  Serial.println(String(buffer) + " " + logOut);

  // if logging turned off then exit
  if (workingLogMode == 0) return;

  File file = ESP_MAIL_DEFAULT_SD_FS.open("/log.txt", FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open Log file for appending");
    return;
  }
  if (!file.println(String(buffer) + " " + logOut)) {
    Serial.println("Log Append failed");
  }
  file.close();
}

// rename log file and start a new one ------------------------------------
void logSwitch() {
  // write a logswitch statement to the logfile
  // rename the current log file
  // create a new log file and write the new date to it
  // email the log file

  logWrite("End of log---Switching");
  delay(100);
  renameFile(ESP_MAIL_DEFAULT_SD_FS, "/log.txt", "/logold.txt");
  logWrite("Start of new log---Switching");
}

// legacy routine to list SD card directory ---------------------------------
void listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}


// startup routine to get config details and then wifi and email details ----------------------------------
void read_SDcard(void) {
  read_datafile(ESP_MAIL_DEFAULT_SD_FS, "/config.txt", CONFIG);   // read config
  read_datafile(ESP_MAIL_DEFAULT_SD_FS, configWifi, WIFI);        // read wifi
  read_datafile(ESP_MAIL_DEFAULT_SD_FS, configEmail, SMTP);       // read email account
  read_datafile(ESP_MAIL_DEFAULT_SD_FS, "/status.txt", STATUS);   // read last status
}
// routine to read a data file and parse according to the type ----------------------------------
int read_datafile(fs::FS &fs, const char *path, int fileType) {
  char temp;
  char test = '\n';
  char buf[200];
  int i = 0;
  int linecount = 0;

  emailLineInput = 0;
  emailRecipientCount = 0;
  emailText = "";

  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return 1;
  }


  while (file.available()) {
    temp = file.read();


    if (temp >= ' ') {
      buf[i] = temp;
      i++;
    } else if (i > 0) {
      buf[i] = 0x0;

      linecount++;
      fileParse(buf, fileType, linecount);
      i = 0;
    }
  }
  buf[i] = 0x0;

  linecount++;
  fileParse(buf, fileType, linecount);

  file.close();
  return 0;
}
// file parse routine to obtain data from SD card files
void fileParse(char *line, int fileType, int lineNo) {
  String workingString;
  char port_temp[6];
  // select based on type of file passed in call
  //    then select of line within type of file
  switch (fileType) {
    case WIFI:
      switch (lineNo) {
        case 1:
          strcpy(ssid, line);     // load wifi SSID to use
          break;
        case 2:
          strcpy(password, line);   // load password to be used
          break;
      }
      break;
    case SMTP:
      switch (lineNo) {
        case 1:
          strcpy(author_email, line);    // load email account to be used
          break;
        case 2:
          strcpy(author_password, line);   // load password for email account
          break;
        case 3:
          strcpy(smtp_host, line);         // load email host to be used
          break;
        case 4:
          workingString = String(line);
          smtp_port = workingString.toInt();    // load port on email host to use
          break;
      }
      break;
    case CONFIG:
      switch (lineNo) {
        case 1:
          strcpy(configHostname, line);     // load hostname to be used
          break;
        case 2:
          strcpy(configWebsite, line);      // load website name to be used NOT YET IMPLEMENTED
          break;
        case 3:
          strcpy(configWifi, line);         // load filename containing wifi credentials
          break;
        case 4:
          strcpy(configEmail, line);        // load filename containing email credentials
          break;
        case 5:
          strcpy(configVerifyEmail, line);  // load filename for verify email details
          break;
        case 6:
          strcpy(configTestEmail, line);     // load filename for test email details
          break;
        case 7:
          strcpy(configAlertEmail, line);    // load filename for alert email details
          break;
        case 8:
          strcpy(configReturnEmail, line);   // // load filename for return email details
          break;
        case 9:
          workingString = String(line);
          workingAlertTimer = workingString.toInt() * 3600;  // load alert timeout in hours and convert to seconds
          break;
        case 10:
          workingString = String(line);
          workingPowerTimer = workingString.toInt() * 3600;  // load power timer in hours and convert to seconds
          break;
        case 11:
          workingString = String(line);
          workingLogTimer = workingString.toInt() * 60;  // load log timer in minutes and convert to seconds
          break;
        case 12:
          workingString = String(line);
          workingLogMode = workingString.toInt();   // load log mode 1 2 3
          break;
        case 13:
          workingString = String(line);
          workingMorningTimerStart = workingString.toInt();   // load morning timer start   NOT YET IMPLEMENTED
          break;
        case 14:
          workingString = String(line);
          workingMorningTimerEnd = workingString.toInt();    // load morning timer end    NOT YET IMPLEMENTED
          break;
        case 15:
          workingString = String(line);
          if (workingString.length() == 7) {
            workingString.toCharArray(workingTestDays, 8);   // load days of week to send test email with log
            break;
          }
          break;
      }
      break;
    case EMAIL:
      logWrite("Input line = " + String(line));

      switch (emailLineInput) {       // parese email files for recipients, subject and content message
        case 0:
          if (line[0] == '<') {
            emailLineInput++;
          } else {
            strcpy(recipients[emailRecipientCount], line);

            logWrite("Recipient " + String(emailRecipientCount) + " = " + String(recipients[emailRecipientCount]));

            emailRecipientCount++;
          }
          break;
        case 1:
          if (line[0] == '<') {
            emailLineInput++;
          } else {
            strcpy(emailSubject, line);

            logWrite("Subject = " + String(emailSubject));
          }
          break;
        case 2:
          emailText = String(emailText + String(line));
          logWrite("Text = " + emailText);
          break;
      }
      break;
    case STATUS:
      switch (lineNo) {       // parse and load saved status details
        case 1:
          workingString = String(line);
          statusTime = workingString.toInt();
          break;
        case 2:
          workingString = String(line);
          statusAlertTime = workingString.toInt();
          break;
        case 3:
          workingString = String(line);
          statusTestTime = workingString.toInt();
          break;
        case 4:
          workingString = String(line);
          alertState = workingString.toInt();
          break;
        case 5:
          workingString = String(line);
          workingLastSeen = workingString.toInt();
          break;
      }
      break;
  }
}

// routine to append to an SD card file ----------------------------------
void appendFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}
// routine to rename an SD card file ----------------------------------
void renameFile(fs::FS &fs, const char *path1, const char *path2) {
  Serial.printf("Renaming file %s to %s\n", path1, path2);
  if (fs.rename(path1, path2)) {
    Serial.println("File renamed");
  } else {
    Serial.println("Rename failed");
  }
}
// routine to delete an SD card file ----------------------------------
void deleteFile(fs::FS &fs, const char *path) {
  Serial.printf("Deleting file: %s\n", path);
  if (fs.remove(path)) {
    Serial.println("File deleted");
  } else {
    Serial.println("Delete failed");
  }
}

// routine to setup SD card for file attachments ----------------------------------
int setup_sdcard() {
  // Serial.begin(115200);
  /* the ESP_MAIL.FS.h header file needed to be updated to add
      #define ESP_MAIL_CARD_TYPE_SD 1
      There are several entries for this in the file but commented out and without
      any clear instructionas as to which should be uncommented
      the sdBegin parameters are now in the order
      ss
      sck
      miso
      mosi
      which differs from other iterations
  */
  ;  /// see note above
     // if (!MailClient.sdBegin(5, 18, 19, 23)) {
     //   Serial.println("Card Mount Failed");
     //   return 1;
     // }
  SD_Card_Mounting();
  return 0;
}
