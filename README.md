# RFID Attendance System using ESP8266 & MFRC522

A wireless RFID-based attendance system built with an ESP8266 (NodeMCU), MFRC522 RFID reader, buzzer, and LEDs. When an RFID card is scanned, the data is read and sent to a Google Spreadsheet via HTTPS.

---

## How It Works

1. ESP8266 connects to Wi-Fi on startup
2. When an RFID card is placed on the reader, it reads the stored name from Block 2
3. The buzzer beeps to confirm the scan
4. The name is sent to a Google Spreadsheet via an HTTPS GET request
5. LEDs indicate Wi-Fi status and data send success/failure

---

## Components

| Component | Quantity |
|-----------|----------|
| ESP8266 NodeMCU (ESP-WROOM-32) | 1 |
| MFRC522 RFID Reader + Card | 1 |
| Buzzer | 1 |
| Red LED | 1 |
| Green LED | 1 |
| 18650 Li-ion Battery (3.7V) | 1 |
| Jumper Wires | As needed |

---

## Wiring / Pin Connections

### MFRC522 → ESP8266

| MFRC522 Pin | ESP8266 Pin |
|-------------|-------------|
| SDA (SS) | D4 |
| SCK | D5 |
| MOSI | D7 |
| MISO | D6 |
| RST | D3 |
| 3.3V | 3.3V |
| GND | GND |

### Other Components

| Component | ESP8266 Pin |
|-----------|-------------|
| Buzzer (+) | D2 |
| Red LED | D1 |
| Green LED | D0 |
| Battery (+) | VIN |
| Battery (−) | GND |

---

## Files

| File | Description |
|------|-------------|
| `attendence.ino` | Main code — reads RFID card and sends data to Google Sheets |
| `card_holder.ino` | Write code — writes a name onto an RFID card (run once per card) |
| `SHEET.java` | Your Wi-Fi credentials and Google Sheets URL to store the data |

---

## Setup Instructions

### Step 1 — Write data to RFID card
- Use `attendence.ino` with an **ESP32** (pins: SS=21, RST=22)
- Edit this line with the cardholder's name (max 16 characters, no spaces):
```cpp
byte blockData[16] = { "YOUR_NAME" };
```
- Upload and scan the card — data will be written to Block 2

### Step 2 — Create `SHEET.java`
Create a file named `SHEET.java` in the same folder as `attendance.ino`:
```cpp
#define WIFI_SSID     "YourWiFiName"
#define WIFI_PASSWORD "YourWiFiPassword"
#define SHEET_URL     "https://script.google.com/macros/s/YOUR_SCRIPT_ID/exec?name="
```

### Step 3 — Set up Google Sheets
1. Create a Google Sheet
2. Go to **Extensions → Apps Script**
3. Deploy a Web App that accepts GET requests and logs data to the sheet
4. Copy the deployment URL into `SHEET.java`

### Step 4 — Upload main code
- Select board: **NodeMCU 1.0 (ESP-12E Module)**
- Upload `attendence.ino` to the ESP8266

---

## LED Status Indicators

| LED | State | Meaning |
|-----|-------|---------|
| Red | Blinking | Connecting to Wi-Fi |
| Green | ON | Wi-Fi connected / Data sent successfully |
| Red | ON | Data send failed |
| Green | Blinking | Data sent after card scan |

## Buzzer

- **2 short beeps** = RFID card scanned successfully

---
