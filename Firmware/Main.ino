#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const byte REFERENCE_LDR_PIN = A0;  // LDR1: clean-panel reference
const byte MONITORED_LDR_PIN = A1;  // LDR2: monitored panel

const byte GREEN_LED_PIN  = 8;
const byte YELLOW_LED_PIN = 9;
const byte RED_LED_PIN    = 10;

// Common PCF8574 backpack address. Change to 0x3F if needed.
LiquidCrystal_I2C lcd(0x27, 16, 2);

const unsigned long DISPLAY_PAGE_MS = 2500;

enum PanelState {
  PANEL_NORMAL,
  DUST_STARTED,
  DUST_SETTLED
};

PanelState classifyDifference(int difference) {
  if (difference <= 10) {
    return PANEL_NORMAL;
  } else if (difference <= 30) {
    // Includes the 11–19 gap and the requested 20–30 range.
    return DUST_STARTED;
  } else {
    return DUST_SETTLED;
  }
}

void setLeds(PanelState state) {
  // Clear all LEDs first so only the current condition LED is on.
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(YELLOW_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);

  if (state == PANEL_NORMAL) {
    digitalWrite(GREEN_LED_PIN, HIGH);
  } else if (state == DUST_STARTED) {
    digitalWrite(YELLOW_LED_PIN, HIGH);
  } else {
    digitalWrite(RED_LED_PIN, HIGH);
  }
}

void showStatus(PanelState state) {
  lcd.clear();

  // Split longer requested messages over the two 16-character rows.
  if (state == PANEL_NORMAL) {
    lcd.setCursor(0, 0);
    lcd.print("Panel is normal");
  } else if (state == DUST_STARTED) {
    lcd.setCursor(0, 0);
    lcd.print("Dust started");
    lcd.setCursor(0, 1);
    lcd.print("settled");
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Dust is fully");
    lcd.setCursor(0, 1);
    lcd.print("settled");
  }
}

void showReadings(int referenceValue, int monitoredValue, int difference) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Ref:");
  lcd.print(referenceValue);
  lcd.print(" Mon:");
  lcd.print(monitoredValue);

  lcd.setCursor(0, 1);
  lcd.print("Difference: ");
  lcd.print(difference);
}

void setup() {
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);

  lcd.init();
  lcd.backlight();
}

void loop() {
  int referenceValue = analogRead(REFERENCE_LDR_PIN);
  int monitoredValue = analogRead(MONITORED_LDR_PIN);
  int difference = abs(referenceValue - monitoredValue);

  PanelState state = classifyDifference(difference);
  setLeds(state);

  // Alternate the full condition message and sensor readings.
  bool showReadingsPage =
      (millis() / DISPLAY_PAGE_MS) % 2 == 1;

  if (showReadingsPage) {
    showReadings(referenceValue, monitoredValue, difference);
  } else {
    showStatus(state);
  }

  delay(200);
}