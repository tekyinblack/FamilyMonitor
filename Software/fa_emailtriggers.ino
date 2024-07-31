/* routines which control email trigger conditions

*/
// Test the alert timeout and return a bool --------------------------------------------------------
int triggerAlert(void) {

  // general alert always operational
  if (workingTime > workingAlertTime) {
    return 1;
  } else {
    return 0;
  }
}

// -------------------------------------------------------------------------------------------------
// set the initial alert time. This may be based on the initial set time or on the last power off recorded time
// if unit has been powered off for more than the powercut time, then it is reset to the default day,
// otherwise the last recorded alert time is used.
void setAlertTime(void) {
  // get current time
  time(&workingTime);
  if ((statusAlertTime + workingPowerTimer) <= workingTime) {
    workingAlertTime = workingTime + workingAlertTimer;
  } else {
    workingAlertTime = statusAlertTime;
  }
  // convert time for printing
  struct tm* timeinfo;
  char buffer[40];
  timeinfo = localtime(&workingAlertTime);
  strftime(buffer, 80, "%A, %B %d %Y %H:%M:%S", timeinfo);
  logWrite("Alert time set to " + String(buffer));
}

// update weekly email counter --------------------------------------------------------------------
void nextTestEmail(void) {
  // calculates next test email based on config file

  // get local time in tm format
  // get day of week as unsigned integer
  // if dow is 0 (ie Sunday) then check for time of day as weekly email sent at lunchtime
  //     if after 12 then dow =7, ie next Sunday
  // addition is dow * 86400 seconds in a day)
  // get time in time format and add addition
  // convert to tm format
  // zero minutes and seconds values and set hour to 12
  // convert to time format
  time_t logTime;
  time_t nextDay;
  int dayCount = 0;
  int testDayCount = 0;
  char check[2] = "0";
  struct tm timeinfo;
  time(&logTime);
  timeinfo = *localtime(&logTime);
  int dayofweek = timeinfo.tm_wday;
  dayCount = dayofweek;
  ;

  while (testDayCount < 7) {
    dayCount = dayCount + 1;
    if (dayCount > 6) dayCount = 0;
    if (workingTestDays[dayCount] != check[0]) break;
    testDayCount++;
  }
  if (testDayCount == 7) {
    // all zeros therefore no test emails
    testEmails = 0;
    workingTestTime = workingTime + 99999999;
    logWrite("Test emails will not be scheduled");
    return;
  }
  testEmails = 1;
  dayCount = dayCount - dayofweek;
  if (dayCount <= 0) dayCount = dayCount + 7;

  nextDay = logTime + dayCount * 86400;
  timeinfo = *localtime(&nextDay);
  timeinfo.tm_hour = 12;
  timeinfo.tm_sec = 0;
  timeinfo.tm_min = 0;
  workingTestTime = mktime(&timeinfo);

  struct tm* nextTime;
  char buffer[40];
  nextTime = localtime(&workingTestTime);
  strftime(buffer, 80, "%A, %B %d %Y %H:%M:%S", nextTime);
  logWrite("Next test email will be sent " + String(buffer));
}

// update email send display LED ----------------------------------------------------------------
void emailSentLED(int state) {
  //process email sent LED
  if (state) {
    digitalWrite(outputEmail, LOW);
    errorLED = 1;
    emailSentTimer = workingTime + emailSentTimeout;
  } else {
    digitalWrite(outputEmail, HIGH);
    errorLED = 0;
  }
}

// read the PIR LED output and set the alert timeout appropriately ------------------------------
void triggerPIR() {
  // Is PIR triggered
  if (digitalRead(inputPIR) == HIGH && statePIR == 0) {

    if (workingLogMode >= 2) logWrite("PIR ON");
    statePIR = 1;
    if (workingTime >= previousPIR + countPIRinterval) {
      previousPIR = workingTime;
      countPIR = 1;
    } else countPIR++;

    if (countPIR >= repeatPIR) {
      workingAlertTime = workingTime + workingAlertTimer;
      workingLastSeen = workingTime;
      previousPIR = workingTime;
      countPIR = 1;
      struct tm* timeinfo;
      char buffer[40];


      timeinfo = localtime(&workingAlertTime);

      strftime(buffer, 80, "%A, %B %d %Y %H:%M:%S", timeinfo);
      if (workingLogMode >= 2) logWrite("Alert time updated to " + String(buffer));
    }
  } else if (digitalRead(inputPIR) == LOW && statePIR == 1) {

    if (workingLogMode >= 2) logWrite("PIR OFF ");
    statePIR = 0;
  }

  // if in morning period....update alert time to end of morning period
  // logic below for example. alert time only needs to be updated to end of morning time once
  // as once seen, standard alerting can resume. Alert time only updated to end of
  // morning time if current alert time does not expire during morning time, that is
  // no extension to existing alert time, so soonest alert time counts.

  // if (workingLastSeen == workingTime) {
  //    checkSeenInMorning = 0;
  //    return 0;
  //  }
  //
  //  // check if morning window specified, if not, skip morning alerts
  //  if (workingMorningTimerStart != 0) {
  //    struct tm  timeinfo;
  //    struct tm  lastSeen;
  //    timeinfo = *localtime(&workingTime);
  //    if (timeinfo.tm_hour < workingMorningTimerStart) {
  //      seenInMorning = 0;
  //    }
  //
  //    // check if currently operational in monitored window, if not, skip morning alerts
  //    if (timeinfo.tm_hour < workingMorningTimerEnd && timeinfo.tm_hour >= workingMorningTimerStart) {
  //      lastSeen = *localtime(&workingLastSeen);
  //      checkSeenInMorning = 1;
  //      // check if last seen time is within morning limits, if so, set seen flag
  //      if (lastSeen.tm_hour < workingMorningTimerEnd && lastSeen.tm_hour >= workingMorningTimerStart) {
  //        seenInMorning = 1;
  //      }
  //    }
  //
  //    // once check period is complete, verify that seen flag set, if not then raise alert
  //    if (timeinfo.tm_hour >= workingMorningTimerEnd)  {
  //      if (checkSeenInMorning) {
  //        if (seenInMorning) {
  //          return 0;
  //        }
  //        else {
  //          return 1;
  //        }
  //      }
  //    }
  //  }
}
// Process the test button press for the verify email -----------------------------------
void triggerVerifyEmail() {
  if (digitalRead(inputButton) != HIGH) {
    // is test button pressed
    //     NO
    //         Set test button flag off
    testButtonFlag = 0;
    //         Set verification flag off
    verificationFlag = 0;
  } else {
    //     YES
    //         Is test test button flag on
    if (!testButtonFlag) {
      //               NO
      //               Set test button flag on
      testButtonFlag = 1;
      //               update test button timer with current time
      testButtonTimer = workingTime + 5;
    } else {
      //               YES
      //               has test button timer expired
      if (workingTime > testButtonTimer) {
        //                      YES
        //                      Is verification flag on
        if (!verificationFlag && stableWiFi && workingTime > emailSentTimer) {
          //                             NO
          //                             Set verification flag on
          verificationFlag = 1;
          //                             Send verification email
          smtpSendVerify();

          logWrite("Sent Verification Email");
          emailSentLED(1);
        }
        //                             YES
        //                             Do nothing
      }
      //               NO
      //               Do nothing
    }
  }
}
