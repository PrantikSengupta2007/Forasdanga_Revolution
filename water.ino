#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ---------------- PIN CONNECTIONS ----------------
#define TRIG_PIN 9
#define ECHO_PIN 10

#define BUZZER 8

#define GREEN_LED 6
#define YELLOW_LED 7
#define RED_LED 5

#define QUALITY_POT A0
#define FLOW_POT A1

// LCD I2C address
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ---------------- SETTINGS ----------------
#define TANK_HEIGHT 20.0   // Tank height in cm

// Thresholds
#define LOW_LEVEL 30
#define HIGH_LEVEL 70

#define QUALITY_GOOD 700
#define QUALITY_MODERATE 400

#define LEAKAGE_THRESHOLD 700


// =================================================
// SETUP
// =================================================
void setup()
{
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(BUZZER, OUTPUT);

  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  Serial.begin(9600);

  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Water Monitoring");
  lcd.setCursor(0, 1);
  lcd.print("System Starting");

  delay(2000);
  lcd.clear();
}


// =================================================
// MEASURE WATER LEVEL
// =================================================
float getDistance()
{
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  float distance = duration * 0.0343 / 2;

  return distance;
}


// =================================================
// CALCULATE WATER LEVEL
// =================================================
int getWaterLevel()
{
  float distance = getDistance();

  // Limit distance
  if (distance > TANK_HEIGHT)
    distance = TANK_HEIGHT;

  if (distance < 0)
    distance = 0;

  // Convert distance into water level percentage
  int level = ((TANK_HEIGHT - distance) / TANK_HEIGHT) * 100;

  if (level < 0)
    level = 0;

  if (level > 100)
    level = 100;

  return level;
}


// =================================================
// GET WATER QUALITY
// =================================================
String getWaterQuality(int value)
{
  if (value >= QUALITY_GOOD)
  {
    return "Good";
  }
  else if (value >= QUALITY_MODERATE)
  {
    return "Moderate";
  }
  else
  {
    return "Poor";
  }
}


// =================================================
// LED AND BUZZER STATUS
// =================================================
void setStatus(int level, int quality, int flow)
{
  // Turn everything OFF first
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, LOW);
  digitalWrite(BUZZER, LOW);

  // Leakage detected
  if (flow > LEAKAGE_THRESHOLD)
  {
    digitalWrite(RED_LED, HIGH);
    digitalWrite(BUZZER, HIGH);
  }

  // Poor water quality
  else if (quality < QUALITY_MODERATE)
  {
    digitalWrite(RED_LED, HIGH);
    digitalWrite(BUZZER, HIGH);
  }

  // Low water level
  else if (level < LOW_LEVEL)
  {
    digitalWrite(YELLOW_LED, HIGH);
  }

  // Moderate quality
  else if (quality < QUALITY_GOOD)
  {
    digitalWrite(YELLOW_LED, HIGH);
  }

  // Everything normal
  else
  {
    digitalWrite(GREEN_LED, HIGH);
  }
}


// =================================================
// MAIN LOOP
// =================================================
void loop()
{
  // -------- WATER LEVEL --------
  int waterLevel = getWaterLevel();

  // -------- WATER QUALITY --------
  int qualityValue = analogRead(QUALITY_POT);

  String quality = getWaterQuality(qualityValue);

  // -------- FLOW / LEAKAGE --------
  // In Wokwi, Potentiometer 2 simulates water flow.
  int flowValue = analogRead(FLOW_POT);

  // -------- STATUS --------
  setStatus(waterLevel, qualityValue, flowValue);

  // -------- LCD DISPLAY --------
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Level: ");
  lcd.print(waterLevel);
  lcd.print("%");

  lcd.setCursor(0, 1);
  lcd.print("Quality: ");
  lcd.print(quality);

  delay(2000);

  // -------- SERIAL MONITOR --------
  Serial.print("Water Level: ");
  Serial.print(waterLevel);
  Serial.println("%");

  Serial.print("Quality Value: ");
  Serial.println(qualityValue);

  Serial.print("Water Quality: ");
  Serial.println(quality);

  Serial.print("Flow Value: ");
  Serial.println(flowValue);

  Serial.println("--------------------");

  // -------- LEAKAGE DISPLAY --------
  if (flowValue > LEAKAGE_THRESHOLD)
  {
    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("!!! WARNING !!!");

    lcd.setCursor(0, 1);
    lcd.print("LEAKAGE DETECTED");

    digitalWrite(RED_LED, HIGH);
    digitalWrite(BUZZER, HIGH);

    delay(2000);
  }
}