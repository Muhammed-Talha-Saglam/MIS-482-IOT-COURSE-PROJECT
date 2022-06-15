#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <ESP_Mail_Client.h>


#define WIFI_SSID "Zyxel_C271"
#define WIFI_PASSWORD "JKHNG7K7D7"
#define AUTHOR_EMAIL "byte04@outlook.com"
#define AUTHOR_PASSWORD "******"
#define RECIPIENT_EMAIL "muhammedtalhasaglam0@gmail.com"
#define SMTP_HOST "smtp.office365.com"
#define SMTP_PORT 587
SMTPSession smtp;

#define LCD_MAX_COLS 16
#define LCD_MAX_ROWS  2
#define moisturePin A0
#define ledRed D8
#define ledGreen D7

int moistureVal;
String message = "";
 
LiquidCrystal_I2C lcd(0x27,LCD_MAX_COLS,LCD_MAX_ROWS);  

int8_t LCD_cursor_col = 0;
int8_t LCD_cursor_row = 0;

int dt = 1000;
boolean isEmailSent = false;

void smtpCallback(SMTP_Status status);

void setup() {
  Serial.begin(115200);

  pinMode(moisturePin, INPUT);
  pinMode(ledRed, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  
  lcd.init();                      
  lcd.backlight();
  resetCursor();

 /// Wifi 
  
  Serial.println();
  Serial.print("Connecting to AP");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(200);
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  smtp.debug(1);

  /* Set the callback function to get the sending results */
  smtp.callback(smtpCallback);
}

void loop() {
  moistureVal = analogRead(moisturePin);
  Serial.print("moistureVal: "); Serial.println(moistureVal);

  if (moistureVal < 200) {
    dry();
  } else {
    watery();
  }
  
  delay(dt);
 }

void watery() {
    writeToLCD("WATERY");
    digitalWrite(ledRed, LOW);  
    digitalWrite(ledGreen, HIGH);
    isEmailSent = false;
}

void dry() {
    writeToLCD("DRY");    
    digitalWrite(ledRed, HIGH);  
    digitalWrite(ledGreen, LOW);
    if (!isEmailSent) {
      sendEmail();        
    }
}

void sendEmail() {
  
    /* Declare the session config data */
  ESP_Mail_Session session;

  /* Set the session config */
  session.server.host_name = SMTP_HOST;
  session.server.port = SMTP_PORT;
  session.login.email = AUTHOR_EMAIL;
  session.login.password = AUTHOR_PASSWORD;
  session.login.user_domain = "";

  /* Declare the message class */
  SMTP_Message message;

  /* Set the message headers */
  message.sender.name = "ESP";
  message.sender.email = AUTHOR_EMAIL;
  message.subject = "Flower Pot Is Dry";
  message.addRecipient("Talha", RECIPIENT_EMAIL);

  /*Send HTML message*/
  String htmlMsg = "<div style=\"color:#2f4468;\"><h1>Hello,</h1><p>- You need to water the flower pot.</p></div>";
  message.html.content = htmlMsg.c_str();
  message.text.charSet = "us-ascii";
  message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

  /* Connect to server with the session config */
  if (!smtp.connect(&session))
    return;

  /* Start sending Email and close the session */
  if (!MailClient.sendMail(&smtp, &message)) {
    Serial.println("Error sending Email, " + smtp.errorReason());    
    return;
  }

   isEmailSent = true;
}

void smtpCallback(SMTP_Status status){
  /* Print the current status */
  Serial.println(status.info());

  /* Print the sending result */
  if (status.success()){
    Serial.println("----------------");
    ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());
    ESP_MAIL_PRINTF("Message sent failled: %d\n", status.failedCount());
    Serial.println("----------------\n");
    struct tm dt;

    for (size_t i = 0; i < smtp.sendingResult.size(); i++){
      /* Get the result item */
      SMTP_Result result = smtp.sendingResult.getItem(i);
      time_t ts = (time_t)result.timestamp;
      localtime_r(&ts, &dt);

      ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
      ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
      ESP_MAIL_PRINTF("Date/Time: %d/%d/%d %d:%d:%d\n", dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);
      ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients);
      ESP_MAIL_PRINTF("Subject: %s\n", result.subject);
    }
    Serial.println("----------------\n");
  }
}

void writeToLCD(String message) {
   resetCursor();
   Serial.print("Writing message: ");Serial.println(message);   
   for (int i=0;i< message.length(); i++){
    char ch = message[i];
    lcd.write(ch);
    increaseLCDCursor();
  }
  
}
void increaseLCDCursor() {
  LCD_cursor_col ++;
  
  if(LCD_cursor_col>=LCD_MAX_COLS){
    LCD_cursor_col = 0;
    LCD_cursor_row++;
  
    if(LCD_cursor_row>=LCD_MAX_ROWS)
      LCD_cursor_row = 0;
    
    lcd.setCursor(LCD_cursor_col,LCD_cursor_row);
    
    for(int i=0;i<LCD_MAX_COLS;i++){
      lcd.write(' ');
    }
  }
  
  lcd.setCursor(LCD_cursor_col,LCD_cursor_row);      
}

void resetCursor() {
  lcd.clear();
  lcd.setCursor(0,0);
  LCD_cursor_col=0;
  LCD_cursor_row=0;
}
