#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

#define TRIG_PIN 9
#define ECHO_PIN 10
#define GREEN_LED 6
#define RED_LED 7
#define BUZZER 8

long duration;
int distance;

int tankHeight = 23;
int waterLevel;

int warningLevel = 3;
int floodLevel = 7;

unsigned long previousSensorTime = 0;
unsigned long previousBlinkTime = 0;
unsigned long previousScrollTime = 0;

bool ledState = false;

const int sensorInterval = 500;

bool introFinished = false;
int scrollIndex = 0;

String line1 = "Taru  Tarun";
String line2 = "Tejash Tushar";

String scroll1 = "                " + line1 + "                ";
String scroll2 = "                " + line2 + "                ";

int readDistanceAvg() {

  long sum = 0;
  int count = 0;

  for (int i = 0; i < 5; i++) {

    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);

    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);

    digitalWrite(TRIG_PIN, LOW);

    duration = pulseIn(ECHO_PIN, HIGH, 30000);

    if (duration > 0) {
      int d = duration * 0.034 / 2;
      sum += d;
      count++;
    }
  }

  if (count == 0) return -1;

  return sum / count;
}

void runIntro() {

  unsigned long currentMillis = millis();

  if (currentMillis - previousScrollTime >= 300) {

    previousScrollTime = currentMillis;

    lcd.setCursor(0,0);
    lcd.print(scroll1.substring(scrollIndex, scrollIndex + 16));

    lcd.setCursor(0,1);
    lcd.print(scroll2.substring(scrollIndex, scrollIndex + 16));

    scrollIndex++;

    if (scrollIndex > scroll1.length() - 16) {

      introFinished = true;
      lcd.clear();
    }
  }
}

void setup() {

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  lcd.begin(16,2);
  lcd.clear();

  Serial.begin(9600);

  lcd.setCursor(0,0);
  lcd.print("Project by-");
}

void loop() {

  unsigned long currentMillis = millis();

  if (!introFinished) {
    runIntro();
    return;
  }

  if (currentMillis - previousSensorTime >= sensorInterval) {

    previousSensorTime = currentMillis;

    distance = readDistanceAvg();

    if (distance == -1) {

      digitalWrite(RED_LED, LOW);
      noTone(BUZZER);

      if (currentMillis - previousBlinkTime >= 300) {

        previousBlinkTime = currentMillis;

        ledState = !ledState;
        digitalWrite(GREEN_LED, ledState);
      }

      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("SENSOR ERROR");

      lcd.setCursor(0,1);
      lcd.print("CHECK SENSOR");

      Serial.println("Sensor Error!");

      return;
    }

    waterLevel = tankHeight - distance;

    if (waterLevel < 0) waterLevel = 0;

    Serial.print("Water Level: ");
    Serial.print(waterLevel);
    Serial.println(" cm");

    lcd.clear();

    lcd.setCursor(0,0);
    lcd.print("LEVEL: ");
    lcd.print(waterLevel);
    lcd.print(" cm");

    // FLOOD
    if (waterLevel >= floodLevel) {

      digitalWrite(RED_LED, HIGH);
      digitalWrite(GREEN_LED, LOW);

      tone(BUZZER,300);

      lcd.setCursor(0,1);
      lcd.print("STATUS: FLOOD");
    }

    // WARNING
    else if (waterLevel >= warningLevel && waterLevel < floodLevel) {

      digitalWrite(RED_LED, LOW);

      int offTime = map(waterLevel, warningLevel, floodLevel, 1000, 80);
      offTime = constrain(offTime, 80, 1000);

      if (currentMillis - previousBlinkTime >= offTime) {

        previousBlinkTime = currentMillis;

        ledState = !ledState;

        digitalWrite(GREEN_LED, ledState);

        if (ledState)
          tone(BUZZER,200);
        else
          noTone(BUZZER);
      }

      lcd.setCursor(0,1);
      lcd.print("STATUS: WARN ");
    }

    // SAFE
    else {

      digitalWrite(GREEN_LED, HIGH);
      digitalWrite(RED_LED, LOW);

      noTone(BUZZER);

      lcd.setCursor(0,1);
      lcd.print("STATUS: SAFE ");
    }
  }
}
