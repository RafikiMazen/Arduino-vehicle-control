#include <LiquidCrystal.h>
#include <Arduino_FreeRTOS.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <RTClib.h>
#define ONE_WIRE_BUS 8 //define the temp pin
LiquidCrystal lcd(10, 9, 6, 5, 13, 12);
RTC_DS1307 RTC;
String drivingMode = " P";

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature sensor
DallasTemperature sensors(&oneWire);

const int SW_pin = 3; // Joystick
const int X_pin = A0; // Joystick-analog pin Joystickconnected to X output
const int Y_pin = A1; // Joystick-analog pin connected to Y output

int T3_BRIDGE1 = 2;
int T3_BRIDGE2 = 4;
int T3_BRIDGE3 = 7;
int T3_BRIDGE4 = A2;
int T3_BRIDGEPWM1 = 3;
//int T3_BRIDGEPWM2=;

void Task1(void *pvParameters);
void Task2(void *pvParameters);
void Task3(void *pvParameters);
void setup() {
  xTaskCreate(Task1, "Task1", 128, NULL, 2, NULL);  //LCD //500
  xTaskCreate(Task2, "Task2", 128, NULL, 5, NULL);  //CAR MOVMENT //500
  pinMode(SW_pin, INPUT);
  digitalWrite(SW_pin, HIGH);
  Serial.begin(115200);
  analogWrite(11, 10); // contrast
  lcd.begin(16, 2);
  Wire.begin();
  RTC.begin();
  RTC.adjust(DateTime(__DATE__, __TIME__));

  // Check to see if the RTC is keeping time.  If it is, load the time from your computer.
  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // This will reflect the time that your sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }

  //Task3--Car Movment
  pinMode(T3_BRIDGE1, OUTPUT);
  pinMode(T3_BRIDGE2, OUTPUT);
  pinMode(T3_BRIDGE3, OUTPUT);
  pinMode(T3_BRIDGE4, OUTPUT);
  pinMode(T3_BRIDGEPWM1, OUTPUT);
  analogWrite(T3_BRIDGEPWM1, 150);
  // Start up the library
  sensors.begin();
  //lcd.print("Apr, 15, 2013"); // print out the date
  vTaskStartScheduler();
}

//this loop will conintue to keep looping so that the time can go as follows
void loop() {}

void Task1 (void *pvParameters) {
  TickType_t xLastWakeTime;
  const TickType_t xDelays = pdMS_TO_TICKS(500);
  xLastWakeTime = xTaskGetTickCount();
  while (1) {
    digitalClockDisplay();
    LCDjoystick(X_pin , Y_pin);
    // Call sensors.requestTemperatures() to issue a global temperature and Requests to all devices on the bus
    sensors.requestTemperatures();
    // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the first IC on the wire
    //  Serial.print(sensors.getTempCByIndex(0));
    lcd.setCursor(0, 1);
    lcd.print(" ");
    lcd.print( sensors.getTempCByIndex(0));
    lcd.print("C");
    vTaskDelayUntil(&xLastWakeTime, xDelays);
  }
}
void Task2(void *pvParameters) {
  TickType_t xLastWakeTime;
  const TickType_t xDelays = pdMS_TO_TICKS(200);
  xLastWakeTime = xTaskGetTickCount();
  while (1) {
    Serial.println("Car Movment");
    if ('D' == (drivingMode[0]) || 'R' == (drivingMode[0])) {
      if ('D' == (drivingMode[0])) {
        digitalWrite(T3_BRIDGE1, HIGH);
        digitalWrite(T3_BRIDGE2, HIGH);
        digitalWrite(T3_BRIDGE3, LOW);
        digitalWrite(T3_BRIDGE4, LOW);
      }
      else {
        digitalWrite(T3_BRIDGE1, LOW);
        digitalWrite(T3_BRIDGE2, LOW);
        digitalWrite(T3_BRIDGE3, HIGH);
        digitalWrite(T3_BRIDGE4, HIGH);
      }
    }
    else {
      digitalWrite(T3_BRIDGE1, HIGH);
      digitalWrite(T3_BRIDGE2, HIGH);
      digitalWrite(T3_BRIDGE3, HIGH);
      digitalWrite(T3_BRIDGE4, HIGH);
    }
    vTaskDelayUntil(&xLastWakeTime, xDelays);
  }
}

void LCDjoystick( int X_pin, int Y_pin)
{
  int x = analogRead(X_pin);
  int y = analogRead(Y_pin);
  lcd.setCursor(15, 1);
  if (y > 850 && x < 600 && x > 300 ) {
    drivingMode = "D";
  }
  if (y < 30 && x < 600 && x > 300) {
    drivingMode = "R";
  }
  if (x < 30 && y < 600 && y > 300 ) {
    drivingMode = "N";
  }
  if (x > 850 && y < 600 && y > 300  ) {
    drivingMode = "P";
  }
  lcd.print(drivingMode);
}

void digitalClockDisplay() {
  lcd.setCursor(0, 0);
  DateTime now = RTC.now();
  lcd.print(now.day(), DEC);
  lcd.print('/');
  lcd.print(now.month(), DEC);
  lcd.print('/');
  lcd.print(now.year(), DEC);
  lcd.print(' ');
  lcd.print(now.hour(), DEC);
  lcd.print(':');
  lcd.print(now.minute(), DEC);

}
