// core smtp send routines 
// these are based on teh supplied examples from the email library

int smtp_send(const char* filename, int logAttach) {
  // logAttach = 0 , no file attached, 1, log.txt as is, 2 log.txt renamed as logold.txt for weekly test

  if (lastError == -2) {   // if last send error was because of the attachment, send without
    logAttach = 0;
  }
  lastError = 0;

  read_datafile(SD, filename, EMAIL);

  smtp.debug(1);

  /* Set the callback function to get the sending results */
  smtp.callback(smtpCallback);

  /* Declare the session config data */
  Session_Config session;

  /* Set the session config */
  session.server.host_name = smtp_host;
  session.server.port = smtp_port;
  session.login.email = author_email;
  session.login.password = author_password;
  session.login.user_domain = F("127.0.0.1");

  /* Declare the message class */
  SMTP_Message message;

  /* Set the message headers */
  message.sender.name = "Family Alert";  //<<=====
  message.sender.email = author_email;
  message.subject = emailSubject;

  for (int i = 0; i < emailRecipientCount; i++) {
    message.addRecipient("Owen Family", recipients[i]);  // <<=====
  }

  message.text.content = emailText.c_str();
  message.text.charSet = "us-ascii";
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;



  if (logAttach) {
    /* The attachment data item */
    SMTP_Attachment att;


    /** Set the inline image info e.g.
       file name, MIME type, file path, file storage type,
       transfer encoding and content encoding
    */
    if (logAttach == 1) {
      att.descr.filename = "log.txt";
      att.descr.mime = "text/plain";
      att.file.path = "/log.txt";
    }
    if (logAttach == 2) {
      att.descr.filename = "logold.txt";
      att.descr.mime = "text/plain";
      att.file.path = "/logold.txt";
    }

    /** The file storage type e.g.
       esp_mail_file_storage_type_none,
       esp_mail_file_storage_type_flash, and
       esp_mail_file_storage_type_sd
    */
    att.file.storage_type = esp_mail_file_storage_type_sd;

    /* Need to be base64 transfer encoding for inline image */
    att.descr.transfer_encoding = Content_Transfer_Encoding::enc_base64;

    /** The orange.png file is already base64 encoded file.
       Then set the content encoding to match the transfer encoding
       which no encoding was taken place prior to sending.
    */
    att.descr.content_encoding = Content_Transfer_Encoding::enc_base64;

    /* Add attachment to the message */
    message.addAttachment(att);
  }


  /* Connect to server with the session config */
  if (!smtp.connect(&session)) {
    sprintf(logbuf, "Connection error, Status Code: %d, Error Code: %d, Reason: %s\n", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
    logWrite(logbuf);
    return 1;
  }
  if (!smtp.isLoggedIn()) {
    logWrite("Error, Not yet logged in.");
  } else {
    if (smtp.isAuthenticated())
      logWrite("Successfully logged in.");
    else
      logWrite("Connected with no Auth.");
  }
  /* Start sending Email and close the session */
  if (!MailClient.sendMail(&smtp, &message, true)) {
    sprintf(logbuf, "Error, Status Code: %d, Error Code: %d, Reason: %s\n", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
    logWrite(logbuf);
    lastError = smtp.errorCode();
    return 2;
  }

  return 0;
}


//Callback function to get the Email sending status -------------------------
void smtpCallback(SMTP_Status status) {

  logWrite(status.info());
  /* Print the sending result */
  if (status.success()) {
    Serial.println("----------------");
    logWrite("Message sent success: " + String(status.completedCount()));
    logWrite("Message sent failed: " + String(status.failedCount()));
    Serial.println("----------------\n");
    struct tm dt;


    for (size_t i = 0; i < smtp.sendingResult.size(); i++) {

      /* Get the result item */
      SMTP_Result result = smtp.sendingResult.getItem(i);

      // In case, ESP32, ESP8266 and SAMD device, the timestamp get from result.timestamp should be valid if
      // your device time was synched with NTP server.
      // Other devices may show invalid timestamp as the device time was not set i.e. it will show Jan 1, 1970.
      // You can call smtp.setSystemTime(xxx) to set device time manually. Where xxx is timestamp (seconds since Jan 1, 1970)

      sprintf(logbuf, "Message No: %d", i + 1);
      logWrite(logbuf);
      sprintf(logbuf, "Status: %s", result.completed ? "success" : "failed");
      logWrite(logbuf);
      sprintf(logbuf, "Date/Time: %s", MailClient.Time.getDateTimeString(result.timestamp, "%B %d, %Y %H:%M:%S").c_str());
      logWrite(logbuf);
      sprintf(logbuf, "Recipient: %s", result.recipients.c_str());
      logWrite(logbuf);
      sprintf(logbuf, "Subject: %s", result.subject.c_str());
      logWrite(logbuf);
    }
    Serial.println("----------------\n");

    // You need to clear sending result as the memory usage will grow up.
    smtp.sendingResult.clear();
  }
}
// send the alert email -------------------------------------------
int smtpSendAlert(void) {

  return smtp_send(configAlertEmail, 1);
  ;
}

// send the return email -------------------------------------------
int smtpSendReturn(void) {

  return smtp_send(configReturnEmail, 1);
}

// send the test email -------------------------------------------
int smtpSendTest(void) {
  int tempReturn;
  // rename the logfiles first
  deleteFile(SD, "/logSent.txt");
  logSwitch();
  tempReturn = smtp_send(configTestEmail, 2);
  renameFile(SD, "/logold.txt", "/logSent.txt");
  return tempReturn;
}

// send the verify email -------------------------------------------
int smtpSendVerify(void) {

  return smtp_send(configVerifyEmail, 1);
}
