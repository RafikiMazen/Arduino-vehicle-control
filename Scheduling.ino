#include <Arduino_FreeRTOS.h>
#include "SoftwareSerial.h"
#include <semphr.h>
void Task1(void *pvParameters);
void Task2(void *pvParameters);
void Task3(void *pvParameters);


SoftwareSerial mySerial(11, 10);
# define Start_Byte 0x7E
# define Version_Byte 0xFF
# define Command_Length 0x06
# define End_Byte 0xEF
# define Acknowledge 0x00 //Returns info with command 0x41 [0x01: info, 0x00: no info]
# define ACTIVATED LOW


const int T1_LineDetectionSensor_PIN = 12;
const int led = 8;
const int led2 = 7;

int T2_sensorPin = A2; // select the input pin for LDR
int T2_led = 13;
int T2_sensorValue = 0;

int buttonNext = 4;
int buttonPause = 2;
int buttonPrevious = 3;
boolean isPlaying = false;
SemaphoreHandle_t xSemaphore = NULL ;

void playFirst();
void pause();
void play();
void playNext();
void playPrevious();
void setVolume(int volume);
void execute_CMD(byte CMD, byte Par1, byte Par2);
void setup(){
  Serial.begin(9600); //sets serial port for communication
  xTaskCreate(Task1, "Task1", 128, NULL, 5, NULL); //Lane Detection
  xTaskCreate(Task2, "Task2", 128, NULL, 4, NULL); //LDR
  xTaskCreate(Task3, "Task3", 128, NULL, 3, NULL); //MP3

  pinMode(T1_LineDetectionSensor_PIN, INPUT);
  pinMode(led, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(T2_led, OUTPUT);

  //
  pinMode(buttonPause, INPUT);
  digitalWrite(buttonPause, HIGH);
  pinMode(buttonNext, INPUT);
  digitalWrite(buttonNext, HIGH);
  pinMode(buttonPrevious, INPUT);
  digitalWrite(buttonPrevious, HIGH);
  if (xSemaphore == NULL) {
    xSemaphore = xSemaphoreCreateMutex();
    if ((xSemaphore) != NULL)
      xSemaphoreGive(xSemaphore);
  }
  //xSemaphoreGive(xSemaphore);
  mySerial.begin (9600);
  delay(1000);
  playFirst();
  //xSemaphoreGive(xSemaphore);
  isPlaying = true;
  setVolume(30);
  vTaskStartScheduler();
}

void loop(){}

void Task1(void *pvParameters) {
  TickType_t xLastWakeTime;
  const TickType_t xDelays = pdMS_TO_TICKS(500);
  xLastWakeTime = xTaskGetTickCount();
  while (1) {
    if (!digitalRead(T1_LineDetectionSensor_PIN)) {
      digitalWrite(led, LOW);
      digitalWrite(led2, LOW);
    }
    else {
      digitalWrite(led, HIGH);
      digitalWrite(led2, HIGH);
    }
    vTaskDelayUntil(&xLastWakeTime, xDelays);
  }
}


void Task2(void *pvParameters) {
  TickType_t xLastWakeTime;
  const TickType_t xDelays = pdMS_TO_TICKS(500);
  xLastWakeTime = xTaskGetTickCount();
  while (1) {
    T2_sensorValue = analogRead(T2_sensorPin); // read the value from the sensor
    Serial.println(T2_sensorValue); //prints the values coming from the sensor on the screen
    delay(100);
    if (T2_sensorValue < 800) {
      digitalWrite(T2_led, HIGH);
    }
    else {
      digitalWrite(T2_led, LOW);
    }
    vTaskDelayUntil(&xLastWakeTime, xDelays);
  }
}
void Task3(void *pvParameters) {
  TickType_t xLastWakeTime;
  const TickType_t xDelays = pdMS_TO_TICKS(500);
  xLastWakeTime = xTaskGetTickCount();
  while (1) {
    if (digitalRead(buttonPause) == ACTIVATED)
    {
      mySerial.print("play/pause");
      if (isPlaying)
      {
        pause();
        isPlaying = false;
        xSemaphoreTake(xSemaphore, portMAX_DELAY);
        //delay(1000);
      } else
      {
        isPlaying = true;
        play();
        xSemaphoreGive(xSemaphore);
      }
    }

    if (isPlaying) {
      if (digitalRead(buttonNext) == ACTIVATED)
      {
        mySerial.print("next");
        playNext();
      }

      if (digitalRead(buttonPrevious) == ACTIVATED)
      {
        mySerial.print("prev");
        playPrevious();
      }
    }
    vTaskDelayUntil(&xLastWakeTime, xDelays);
  }
}

void playFirst()
{
  mySerial.print("play first");
  execute_CMD(0x3F, 0, 0);
  delay(500);
  setVolume(20);
  delay(500);
  execute_CMD(0x11, 0, 1);
  delay(500);
  xSemaphoreGive(xSemaphore);
}

void pause()
{

  execute_CMD(0x0E, 0, 0);
  delay(500);

}

void play()
{
  execute_CMD(0x0D, 0, 1);
  delay(500);
}

void playNext()
{
  xSemaphoreTake(xSemaphore, portMAX_DELAY);
  execute_CMD(0x01, 0, 1);
  delay(500);
  xSemaphoreGive(xSemaphore);
}

void playPrevious()
{
  xSemaphoreTake(xSemaphore, portMAX_DELAY);
  execute_CMD(0x02, 0, 1);
  delay(500);
  xSemaphoreGive(xSemaphore);
}

void setVolume(int volume)
{
  execute_CMD(0x06, 0, volume); // Set the volume (0x00~0x30)
  delay(2000);
}

void execute_CMD(byte CMD, byte Par1, byte Par2)
// Excecute the command and parameters
{
  // Calculate the checksum (2 bytes)
  word checksum = -(Version_Byte + Command_Length + CMD + Acknowledge + Par1 + Par2);
  // Build the command line
  byte Command_line[10] = { Start_Byte, Version_Byte, Command_Length, CMD, Acknowledge,
                            Par1, Par2, highByte(checksum), lowByte(checksum), End_Byte
                          };
  //Send the command line to the module
  for (byte k = 0; k < 10; k++)
  {
    mySerial.write( Command_line[k]);
  }
}
