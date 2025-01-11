#include <Wire.h>
#include <RTClib.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Keypad.h>

// RTC setup
RTC_DS1307 rtc;

// DS18B20 setup
#define ONE_WIRE_BUS 4 // GPIO pin for DS18B20
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Sensor pins
#define AO2YYUW_PIN 34
#define EC_PIN 35
#define PH_PIN 34

// Relay pins
#define RELAY1_PIN 16
#define RELAY2_PIN 17
#define RELAY3_PIN 25
#define RELAY4_PIN 26

// Keypad setup
const byte ROWS = 4; // Four rows
const byte COLS = 4; // Four columns
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {19, 18, 5, 17}; // Connect to the row pins
byte colPins[COLS] = {16, 4, 0, 2};   // Connect to the column pins
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Password setup
const String correctPassword = "1234";
String enteredPassword = "";
bool isUnlocked = false;

void setup() {
  Serial.begin(115200);

  // Initialize RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Initialize DS18B20
  sensors.begin();

  // Initialize relays
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(RELAY3_PIN, OUTPUT);
  pinMode(RELAY4_PIN, OUTPUT);
  digitalWrite(RELAY1_PIN, LOW);
  digitalWrite(RELAY2_PIN, LOW);
  digitalWrite(RELAY3_PIN, LOW);
  digitalWrite(RELAY4_PIN, LOW);

  Serial.println("System initialized. Enter password to unlock.");
}

void loop() {
  if (!isUnlocked) {
    handlePasswordInput();
    return;
  }

  // Read sensors
  float temperature = readTemperature();
  float oxygen = analogRead(AO2YYUW_PIN) * (5.0 / 4095.0); // Placeholder calculation
  float ec = analogRead(EC_PIN) * (5.0 / 4095.0);           // Placeholder calculation
  float ph = analogRead(PH_PIN) * (5.0 / 4095.0);           // Placeholder calculation

  // Get current time
  DateTime now = rtc.now();
  int currentHour = now.hour();
  int currentMinute = now.minute();

  // Control pumps based on time and pH
  controlPumps(currentHour, currentMinute, ph);

  // Debug output
  Serial.print("Temp: "); Serial.print(temperature);
  Serial.print(" | O2: "); Serial.print(oxygen);
  Serial.print(" | EC: "); Serial.print(ec);
  Serial.print(" | pH: "); Serial.println(ph);

  delay(1000); // Delay for stability
}

void handlePasswordInput() {
  char key = keypad.getKey();
  if (key) {
    if (key == '#') {
      if (enteredPassword == correctPassword) {
        isUnlocked = true;
        Serial.println("Access granted.");
      } else {
        Serial.println("Incorrect password.");
      }
      enteredPassword = ""; // Reset password
    } else if (key == '*') {
      enteredPassword = ""; // Clear entered password
      Serial.println("Password cleared.");
    } else {
      enteredPassword += key;
      Serial.print("Entered: ");
      Serial.println(enteredPassword);
    }
  }
}

float readTemperature() {
  sensors.requestTemperatures();
  return sensors.getTempCByIndex(0);
}

void controlPumps(int hour, int minute, float ph) {
  // Example: Activate pumps at specific times and based on pH values
  if (hour == 8 && minute == 0 && ph < 7.0) {
    digitalWrite(RELAY1_PIN, HIGH);
    delay(1000);
    digitalWrite(RELAY1_PIN, LOW);
  }

  if (hour == 12 && minute == 0 && ph >= 7.0) {
    digitalWrite(RELAY2_PIN, HIGH);
    delay(1000);
    digitalWrite(RELAY2_PIN, LOW);
  }

  if (hour == 16 && minute == 0) {
    digitalWrite(RELAY3_PIN, HIGH);
    delay(1000);
    digitalWrite(RELAY3_PIN, LOW);
  }

  if (hour == 20 && minute == 0) {
    digitalWrite(RELAY4_PIN, HIGH);
    delay(1000);
    digitalWrite(RELAY4_PIN, LOW);
  }
}
