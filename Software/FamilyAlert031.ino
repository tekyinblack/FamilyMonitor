// Compile with MH ET LIVE ESP32 Minikit
//To use send Email for Gmail to port 465 (SSL), less secure app option should be enabled. https://myaccount.google.com/lesssecureapps?pli=1

#include <Arduino.h>
#include <WiFi.h>
#include "time.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include <ESP_Mail_Client.h>

// Provide the SD card interfaces setting and mounting
// SDHelper.h is provided by the email library and must be updated to map the SD card access pins
// this is required if the monitor is to send attached files.
#include <extras/SDHelper.h>
#include "FS.h"
#include "SPI.h"

int triggerAlert(void);
void setAlertTime(void);
void nextTestEmail(void);
void emailSentLED(int state);
void triggerPIR();
void triggerVerifyEmail();
void dumpVars(void);
void waitWiFi(void);
void badSD(void);
void badConfig(void);
void noTime(void);
void saveStatusToSD(void);
void logWrite(String logOut);
void logSwitch();
void read_SDcard(void);
int read_datafile(fs::FS& fs, const char* path, int fileType);
void fileParse(char* line, int fileType, int lineNo);
int smtp_send(const char* filename, int logAttach);
void smtpCallback(SMTP_Status status);
int smtpSendAlert(void);
int smtpSendReturn(void);
int smtpSendTest(void);
int smtpSendVerify(void);
void printLocalTime();
int time_setup(void);
void displayWiFi();
void reconnectWiFi();


time_t workingTime;
time_t workingLastSeen;
time_t workingAlertTime;
time_t workingTestTime;
time_t emailSentTimer;
time_t testButtonTimer;
time_t saveStatusTime;
struct tm timeFormat;

time_t statusTime;
time_t statusAlertTime;
time_t statusTestTime;
char statusAlert[15] = " ";

time_t wifiTimer;    // wifi timer to ensure wifi stable after reconnection to prevent connection yoyo

time_t bootTimer;    // activity timer for after boot

time_t lastEmail;    // timer variable to prevent send email runaway loop

char recipients[10][50];       // email recipient list
int emailLineInput = 0;
int emailRecipientCount = 0;
char emailSubject[50];
String emailText;

char logbuf[200];

int alertState = 0;
#define NORMAL 0
#define ALERTRAISED 1
#define ALERTSENT 2

// dummy network credentials
char ssid[33] = "DOESNOTEXIST";
char password[33] = "PASSWORD";

const int inputButton = 16;    // test email button
const int inputPIR = 17;       // input from PIR LED
const int outputWiFi = 21;     // output to wifi LED, active low 
const int outputEmail = 22;    // output to email and error LED, active low

// Flags
int statePIR = 0;
int testButtonFlag = 0;
int verificationFlag = 0;
int timeSet = 0;
int testSet = 0;
int errorLED = 0;
int statusWiFi = 0;
int stableWiFi = 0;
int oldStatusWiFi = 0;
//int alertSent = 0;
int testEmails = 0;          // send test emails flag
int seenInMorning = 0;       // flag to indicate activity during morning checks
int checkSeenInMorning = 0;  //flag to indicatecheck for morning activity required

int lastError = 0; // last email send error

// Reconnect timers
#define RECONNECT_BASE 30000
#define RECONNECT_INC 30000
#define RECONNECT_MAX 600000
long currentReconnect = 0;
long previousReconnect = 0;
long reconnectInterval = RECONNECT_BASE;

// timer values defined as variables to ease conversion to loaded parameters
int emailSentTimeout = 60;    // timer to light email send LED
int wifiStableTime = 60;      // timer to determine wifi stable
int bootDormantTime = 300;    // time monitor is dormant after boot

// PIR detect windows
int countPIR = 0;
int repeatPIR = 3;
int countPIRinterval = 60;
long previousPIR = 0;
long currentPIR = 0;


char smtp_host[50] = "not real email host";  
int smtp_port = 465;

// Dummy email sign in credentials

char author_email[50] = "not real email";
char author_password[50] = "not real email password";

// config variables
char configHostname[20] = "FamilyMonitor";  // hostname
char configWebsite[20];                     // website folder
char configWifi[20];                        // wifi security
char configEmail[20];                       // email security
char configVerifyEmail[20];                 // verify operation email details file
char configTestEmail[20];                   // test email file
char configAlertEmail[20];                  // alert email file
char configReturnEmail[20];                 // return email file
long workingAlertTimer = 86400;             // alert timer as integer
long workingPowerTimer = 21600;             // Power Outage timer as integer
long workingLogTimer = 900;                 // log timer as an integer
long workingMorningTimerStart = 6;          // Start morning timer as an integer
long workingMorningTimerEnd = 12;           // End morning timer as an integer
long workingLogMode = 2;                    // logmode as an integer
#define NOLOG 0                             // nothing written to log file
#define NOACTIVITY 1                        // only admin data written to log file
#define ALLLOG 2                            // all data written to log file
char workingTestDays[10] = "1000000";       // which day of week to send log files, Sunday =1



/* The SMTP Session object used for Email sending */
SMTPSession smtp;

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status);

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);  //disable brownout detector

  // assign external pins
  pinMode(inputButton, INPUT);    // input button to trigger sending of test emails
  pinMode(inputPIR, INPUT);       // input from PIR
  pinMode(outputWiFi, OUTPUT);    // output to wifi dual led
  pinMode(outputEmail, OUTPUT);   // output to error/status indicator

  digitalWrite(outputWiFi, HIGH);   // set wifi off
  digitalWrite(outputEmail, HIGH);  // set email off

  // start serial logging
  Serial.begin(115200);
  Serial.println("Starting....");
  delay(1000);

  // read SD card, if this fails then monitor will fail to run....
  Serial.println("Reading SD Card....");

  while (setup_sdcard()) {
    badSD();
  }
  read_SDcard();
  logWrite("Monitor starting");

  //log the program version and loaded parameters
  logWrite(String(__FILE__) + " " + String(__DATE__) + " " + String(__TIME__));
  dumpVars();
  //


  // Start wifi connection
  logWrite("Connecting to SSID " + String(ssid) + " as " + String(configHostname));
  //WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);   <<<<=======removed due to wifi failing to acquire a new address
  WiFi.setHostname(configHostname);  //define hostname from input parameters

  WiFi.begin(ssid, password);    // start wifi
  while (WiFi.status() != WL_CONNECTED) {      // loop until wifi connected
    Serial.print(".");  //<------ dont write this to log
    waitWiFi();   // flash wifi LED until connected
  }
  

  displayWiFi();  // update wifi LED as per status

  // hold here until time is setup, until rtc time set, monitor can't function
  // time_setup also called by displayWiFi but may fail if internet connection not yet setup

  while (!time_setup() && !timeSet) {
    noTime();

    // keep wifi display up to date in case it's dropping out
    displayWiFi();
  }


  time(&workingTime);  // load rtc time into variable

  wifiTimer = workingTime + wifiStableTime;   // set stable wifi timer. Unit waits until wifi stable before attempting emails

  bootTimer = workingTime + bootDormantTime;  // set stable operation timer. Unit waits for fixed time before raising alerts after power on


  // set the alert time for startup based on loaded time from status, power timer and current time

  setAlertTime();

  // if test email late send
  if (statusTestTime < workingTime) {
    //        invoke test email
    if (!smtpSendTest()) {
      logWrite("Sent Test Email");
      emailSentLED(1);
    } else {
      logWrite("Failed to send Test Email!!!!");
    }
  }
  nextTestEmail();

  // load test time from saved status
  // workingTestTime = statusTestTime;

  // save updated alert time
  saveStatusToSD();
  // update next save status time
  saveStatusTime = workingTime + workingLogTimer;
}


// Main loop, cycle repeatedly recording changes in PIR status,
//  checking if any timers haven't expired, and logging current time and actions
// the idea is to keep this looking as simple as possible with actual processing in functions
void loop() {
  // get time from cpu rtc for this iteration of the loop
  time(&workingTime);

  // update wifi status display
  displayWiFi();

  // wait until wifi is stable for set time before flagging suitable for use
  if (stableWiFi == 0 && statusWiFi == 1) {
    if (wifiTimer < workingTime) {
      stableWiFi = 1;
      logWrite("WiFi judged to be stable");
    }
    if (wifiTimer == 0) {
      wifiTimer = workingTime;
    }
  }

  // Reconnect Wifi
  // checks if wifi down and attempt a reconnection on a timed schedule
  reconnectWiFi();

  //          Has mail sent LED timer expired
  //                   YES
  //                   Reset mail set flag and turn off LED

  if (workingTime > emailSentTimer) {
    emailSentLED(0);
  }

  // read the PIR status, the LED is always driven by the PIR detector, not the cpu
  // if concurrent PIR readings are observed then the alert time is updated.
  // the alert state is updated if concurrent PIR readings haven't been observed 
  // and the time exceeds the current alert time
  triggerPIR();

  // state machine
  // if status is 0-Normal, check if alert time exceeded. If so, change to alert state
  // if status is 1-alert raised, check if alert still valid, if not, reset alert, otherwise send an alert email
  // if status is 2-alert sent, and alert email has been sent, check if alert is still valid, if it isn't send a return email and change status to Normal
  switch (alertState) {
    case 0:  // NORMAL
      if (triggerAlert()) {
        //change status
        alertState = ALERTRAISED;
        //save new status
        saveStatusToSD();
        logWrite("Status changed to ALERTRAISED-----------------");
      }
      break;

    case 1:  // ALERTRAISED
      if (!triggerAlert()) {   // if no longer valid, reset status
        alertState = NORMAL;
        //save new status
        saveStatusToSD();
        logWrite("Status changed to NORMAL-----------------");
        break;
      }

      // if alert still valid, and wifi is stable and the unit has been booted for a while, send alert email
      if (triggerAlert() && stableWiFi && bootTimer < workingTime && workingTime > emailSentTimer ) { 
        // send Alert Email
        if (!smtpSendAlert()) {
          // if successful, update and save staus
          alertState = ALERTSENT;
          saveStatusToSD();
          logWrite("Status changed to ALERTRAISED------------------");
          logWrite("Sent Alert Email!!!!!");
          // turn on email send LED
          emailSentLED(1);
          // alertSent = true ;
          //        checkSeenInMorning = 0;  // cancel morning check once complete
        }

        else {
          // otherwise log failure
          logWrite("Failed to send Alert Email!!!!!------------------");
        }
      }
      break;
    case 2:  // ALERTSENT
      // if the alert timer has been reset because of PIR activity, and the
      // alert message has been sent, send the return email to indicate someone is in the property
      if (!triggerAlert() && stableWiFi && workingTime > emailSentTimer) {
         // send return email
        if (!smtpSendReturn()) {
          // if return email send ok, update status
          alertState = NORMAL;
          // save new status
          saveStatusToSD();
          logWrite("Status changed to NORMAL-----------------");
        } else {
          // log error message
          logWrite("Failed to send Return Email!!!!!----------------------");
        }
      }
      break;
  }


  // compare current time with test email datetime
  // if current time gt time out
  if (workingTime > workingTestTime && testEmails && stableWiFi && workingTime > emailSentTimer) {
    //        invoke test email
    if (!smtpSendTest()) {

      logWrite("Sent Test Email");
      emailSentLED(1);
      //        calculate next test time
      nextTestEmail();
    } else {
      logWrite("Failed to send Test Email!!!!");
    }
  }

  // test if the email push button has been pressed for long enough and if so, send email
  triggerVerifyEmail();

  // test if the alerttime is due to be saved to the SD card and if so, save it
  if (workingTime > saveStatusTime) {
    // save status
    saveStatusToSD();
    saveStatusTime = workingTime + workingLogTimer;
  }
}
