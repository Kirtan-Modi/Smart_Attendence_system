// ─────────────────────────────────────────────
//  Libraries
// ─────────────────────────────────────────────
#include <SPI.h>                        
#include <MFRC522.h>                    
#include <Arduino.h>                    
#include <ESP8266WiFi.h>                
#include <ESP8266WiFiMulti.h>           
#include <ESP8266HTTPClient.h>          
#include <WiFiClient.h>                 
#include <WiFiClientSecureBearSSL.h>    
#include "secrets.h"                    

// ─────────────────────────────────────────────
//  Pin Definitions
// ─────────────────────────────────────────────
#define RST_PIN D3      // MFRC522 reset pin
#define SS_PIN  D4      // MFRC522 slave select (SDA) pin
#define BUZZER  D2      // Buzzer pin
#define LED_R   D1      // Red LED pin  (error / no Wi-Fi)
#define LED_G   D0      // Green LED pin (success / Wi-Fi connected)

// ─────────────────────────────────────────────
//  Global Variables
// ─────────────────────────────────────────────
String RFID;                            // Stores the name read from the RFID card
MFRC522 mfrc522(SS_PIN, RST_PIN);       
MFRC522::MIFARE_Key key;                
ESP8266WiFiMulti WiFiMulti;             
MFRC522::StatusCode status;             

int  blockNum     = 2;                  
byte bufferLen    = 18;                 
byte readBlockData[18];                 

String data2;                         

// Google Apps Script Web App URL — replace with your deployed script URL
const String data1 = "https://script.google.com/macros/s/YOUR_DEPLOYMENT_ID/exec?name=";

// ─────────────────────────────────────────────
//  Function Declarations
// ─────────────────────────────────────────────
void ReadDataFromBlock(int blockNum, byte readBlockData[]);

// ─────────────────────────────────────────────
//  Setup — runs once on power ON
// ─────────────────────────────────────────────
void setup() {
  Serial.begin(9600);       // Start serial monitor at 9600 baud
  delay(100);

  pinMode(BUZZER, OUTPUT);
  pinMode(LED_R,  OUTPUT);
  pinMode(LED_G,  OUTPUT);

  digitalWrite(BUZZER, LOW);

  
  WiFi.mode(WIFI_STA);

  // Add your Wi-Fi network — replace with your SSID and password
  WiFiMulti.addAP("YourWiFiName", "YourWiFiPassword");

  Serial.println("Connecting to Wi-Fi...");

  // Wait until Wi-Fi is connected — blink red LED while connecting
  while (WiFiMulti.run() != WL_CONNECTED) {
    digitalWrite(LED_G, LOW);
    digitalWrite(LED_R, HIGH);
    delay(100);
    digitalWrite(LED_R, LOW);
    delay(100);
  }

  // Wi-Fi connected — turn on green LED
  Serial.println("Wi-Fi Connected!");
  digitalWrite(LED_R, LOW);
  digitalWrite(LED_G, HIGH);

  // Initialize SPI bus for MFRC522
  SPI.begin();
}

// ─────────────────────────────────────────────
//  Loop — runs repeatedly after setup()
// ─────────────────────────────────────────────
void loop() {

  // Keep green LED ON when connected
  digitalWrite(LED_G, HIGH);

  // ── Check Wi-Fi connection ──────────────────
  if (WiFiMulti.run() != WL_CONNECTED) {
    Serial.println("Wi-Fi lost. Reconnecting...");

    // Blink red LED while reconnecting
    while (WiFiMulti.run() != WL_CONNECTED) {
      digitalWrite(LED_G, LOW);
      digitalWrite(LED_R, HIGH);
      delay(100);
      digitalWrite(LED_R, LOW);
      delay(100);
    }

    // Reconnected — switch back to green LED
    Serial.println("Wi-Fi Reconnected!");
    digitalWrite(LED_R, LOW);
    digitalWrite(LED_G, HIGH);
  }

  mfrc522.PCD_Init();

  // Exit loop if no new RFID card is detected
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Exit loop if card serial cannot be read
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // ── Read Data from RFID Card ────────────────
  Serial.println();
  Serial.println(F("Reading data from RFID card..."));

  ReadDataFromBlock(blockNum, readBlockData);

  Serial.println();
  Serial.print(F("Data in Block "));
  Serial.print(blockNum);
  Serial.print(F(" --> "));

  // Clear previous RFID string and build new one from raw bytes
  RFID.remove(0, RFID.length());
  int j = 0;
  while (int(readBlockData[j]) > 0) {
    RFID.concat(char(readBlockData[j]));
    j++;
  }
  Serial.println(RFID);

  // ── Buzzer Beep — confirms card scan ───────
  // Two short beeps
  digitalWrite(BUZZER, HIGH); delay(200);
  digitalWrite(BUZZER, LOW);  delay(200);
  digitalWrite(BUZZER, HIGH); delay(200);
  digitalWrite(BUZZER, LOW);

  // Turn off green LED while sending data
  digitalWrite(LED_G, LOW);

  // ── Send Data to Google Spreadsheet ────────
  if (WiFiMulti.run() == WL_CONNECTED) {

    // Create a secure HTTPS client (required for Google Scripts)
    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);

    // Skip SSL certificate verification (needed for Google redirects)
    client->setInsecure();

    // Build full URL: base URL + RFID name
    data2 = data1 + RFID;
    data2.trim();   // Remove any trailing spaces or newlines
    Serial.println("Sending to: " + data2);

    HTTPClient https;
    Serial.print(F("[HTTPS] Connecting...\n"));

    if (https.begin(*client, (String)data2)) {

      Serial.print(F("[HTTPS] Sending GET request...\n"));

      // Send the GET request and get response code
      int httpCode = https.GET();

      if (httpCode > 0) {
        // Request was successful
        Serial.printf("[HTTPS] Response code: %d\n", httpCode);

        // Blink green LED twice to confirm data sent
        digitalWrite(LED_R, LOW);
        digitalWrite(LED_G, HIGH); delay(200);
        digitalWrite(LED_G, LOW);  delay(200);
        digitalWrite(LED_G, HIGH); delay(200);
        digitalWrite(LED_G, LOW);

      } else {
        // Request failed — turn on red LED
        Serial.printf("[HTTPS] Request failed: %s\n", https.errorToString(httpCode).c_str());
        digitalWrite(LED_G, LOW);
        digitalWrite(LED_R, HIGH);
      }

      https.end();    // Close the connection
      delay(500);     // Short delay before next scan

    } else {
      // Could not connect to the server
      Serial.println("[HTTPS] Unable to connect to server");
      digitalWrite(LED_G, LOW);
      digitalWrite(LED_R, HIGH);
    }
  }
}


void ReadDataFromBlock(int blockNum, byte readBlockData[]) {

  // Set default authentication key (0xFF x6 is factory default)
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  // Authenticate the block before reading
  status = mfrc522.PCD_Authenticate(
    MFRC522::PICC_CMD_MF_AUTH_KEY_A,
    blockNum,
    &key,
    &(mfrc522.uid)
  );

  if (status != MFRC522::STATUS_OK) {
    Serial.print("Authentication failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  Serial.println("Authentication successful");

  // Read 16 bytes from the authenticated block
  status = mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen);

  if (status != MFRC522::STATUS_OK) {
    Serial.print("Read failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  Serial.println("Block read successfully");
}
