// ─────────────────────────────────────────────
//  RFID Card Writer — ESP32
//  Writes a cardholder name to Block 2 of a
//  MIFARE 1K RFID card, then reads it back
//  to verify the write was successful.
// ─────────────────────────────────────────────

#include <SPI.h>
#include <MFRC522.h>

// Pin Definitions (ESP32)
constexpr uint8_t RST_PIN = 22;
constexpr uint8_t SS_PIN  = 21;

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
MFRC522::StatusCode status;

int  blockNum  = 2;                       
byte blockData[16] = { "Kirtan" };        
byte bufferLen = 18;                    
byte readBlockData[18];           

// Function declarations
void WriteDataToBlock(int blockNum, byte blockData[]);
void ReadDataFromBlock(int blockNum, byte readBlockData[]);


void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("Scan a MIFARE 1K Tag to write data...");
}


void loop() {

  // Set default authentication key (factory default is 0xFF x6)
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  // Wait for a card to be placed on the reader
  if (!mfrc522.PICC_IsNewCardPresent()) { return; }
  if (!mfrc522.PICC_ReadCardSerial())   { return; }

  // ── Print Card Info ─────────────────────────
  Serial.println("\n**Card Detected**");

  Serial.print(F("Card UID:"));
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }

  Serial.print(F("\nPICC type: "));
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.println(mfrc522.PICC_GetTypeName(piccType));

  // ── Write & Verify ──────────────────────────
  Serial.println("\nWriting to Data Block...");
  WriteDataToBlock(blockNum, blockData);

  // Read back to confirm data was written correctly
  Serial.println("\nReading from Data Block...");
  ReadDataFromBlock(blockNum, readBlockData);

  Serial.print("\nData in Block ");
  Serial.print(blockNum);
  Serial.print(" --> ");
  for (int j = 0; j < 16; j++) {
    Serial.write(readBlockData[j]);
  }
  Serial.println();
}


// ─────────────────────────────────────────────
//  WriteDataToBlock
//  Authenticates and writes 16 bytes of data
//  to the specified block on the RFID card
// ─────────────────────────────────────────────
void WriteDataToBlock(int blockNum, byte blockData[]) {
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Authentication failed for Write: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  Serial.println("Authentication success");

  status = mfrc522.MIFARE_Write(blockNum, blockData, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Writing to Block failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  Serial.println("Data was written into Block successfully");
}


// ─────────────────────────────────────────────
//  ReadDataFromBlock
//  Authenticates and reads 16 bytes of data
//  from the specified block on the RFID card
// ─────────────────────────────────────────────
void ReadDataFromBlock(int blockNum, byte readBlockData[]) {
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Authentication failed for Read: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  Serial.println("Authentication success");

  status = mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Reading failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  Serial.println("Block was read successfully");
}
