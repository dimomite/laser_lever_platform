#include <Arduino.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "remoteserver.hpp"

#define DEBUG_INPUT (false)
#define DEBUG_OUT_ACTIONS (false)

#define LED_ALARM (2)

#define IN_LEFT_MAX (25)
#define IN_RIGHT_MAX (26)

#define IN_MOVE_LEFT (5)
#define IN_MOVE_RIGHT (16)
#define IN_TURN_CW (4)
#define IN_TURN_CCW (0)

#define OUT_A (14)
#define OUT_B (12)
#define OUT_C (13)
#define OUT_D (15)

static Adafruit_SSD1306 display(128, 64, &Wire, -1);

static void startServerTask()
{
  xTaskCreate(
      remoteContolServerTaks, // task function
      "extraTask",            // task name
      10000,                  // stack size in words
      nullptr,                // args
      2,                      // priority
      nullptr                 // task handler
  );
  delay(2000);
}

static void printCore()
{
  Serial.print("Running on core #");
  Serial.println(xPortGetCoreID());
}

void setup()
{
  Serial.begin(115200);
  printCore();

  Wire.begin(5, 4);
  if (display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false))
  {
    Serial.println("Connected to the display");
    display.clearDisplay();
    display.drawCircle(16, 16, 8, WHITE);
    display.fillCircle(28 , 16, 8, WHITE);
    display.display();
  }
  else
  {
    Serial.println("Cound not connect to the display");
    while (true)
    {
    }
  }

  pinMode(LED_ALARM, OUTPUT);

  pinMode(OUT_A, OUTPUT);
  pinMode(OUT_B, OUTPUT);
  pinMode(OUT_C, OUTPUT);
  pinMode(OUT_D, OUTPUT);

  pinMode(IN_LEFT_MAX, INPUT_PULLUP);
  pinMode(IN_RIGHT_MAX, INPUT_PULLUP);

  pinMode(IN_MOVE_LEFT, INPUT_PULLUP);
  pinMode(IN_MOVE_RIGHT, INPUT_PULLUP);
  pinMode(IN_TURN_CW, INPUT_PULLUP);
  pinMode(IN_TURN_CCW, INPUT_PULLUP);

  startServerTask();
}

static bool isAlarmOn = false;

static void moveLeft()
{
  digitalWrite(OUT_C, LOW);
  digitalWrite(OUT_D, HIGH);
#if (DEBUG_OUT_ACTIONS)
  Serial.println("Moving left...");
#endif
}

static void moveRight()
{
  digitalWrite(OUT_C, HIGH);
  digitalWrite(OUT_D, LOW);
#if (DEBUG_OUT_ACTIONS)
  Serial.println("Moving right...");
#endif
}

static void stopMoving()
{
  digitalWrite(OUT_C, LOW);
  digitalWrite(OUT_D, LOW);
}

static void turnCW()
{
  digitalWrite(OUT_A, LOW);
  digitalWrite(OUT_B, HIGH);
#if (DEBUG_OUT_ACTIONS)
  Serial.println("Turning CW...");
#endif
}

static void turnCCW()
{
  digitalWrite(OUT_A, HIGH);
  digitalWrite(OUT_B, LOW);
#if (DEBUG_OUT_ACTIONS)
  Serial.println("Turning CCW...");
#endif
}

static void stopTurning()
{
  digitalWrite(OUT_A, LOW);
  digitalWrite(OUT_B, LOW);
}

void loop()
{
  auto isLeftMax = digitalRead(IN_LEFT_MAX) == HIGH;
  auto isRightMax = digitalRead(IN_RIGHT_MAX) == HIGH;
  auto isMoveLeft = digitalRead(IN_MOVE_LEFT) == LOW;
  auto isMoveRight = digitalRead(IN_MOVE_RIGHT) == LOW;
  auto isTurnCW = digitalRead(IN_TURN_CW) == LOW;
  auto isTurnCCW = digitalRead(IN_TURN_CCW) == LOW;

#if (DEBUG_INPUT)
  if (isLeftMax)
  {
    Serial.println("Left max is reached");
  }
  if (isRightMax)
  {
    Serial.println("Right max is reached");
  }

  if (isMoveLeft)
  {
    Serial.println("Move left command received");
  }
  if (isMoveRight)
  {
    Serial.println("Move right command received");
  }

  if (isTurnCW)
  {
    Serial.println("Turn CW command received");
  }
  if (isTurnCCW)
  {
    Serial.println("Turn CCW command received");
  }
#endif

  isAlarmOn = false;
  if (isLeftMax && isRightMax)
  {
    Serial.println("!!! End stops error, both are active !!!");
    isAlarmOn = true;
  }
  if (isMoveLeft && isMoveRight)
  {
    Serial.println("!!! Move left and right commands are received at the same time !!!");
    isAlarmOn = true;
  }
  if (isTurnCW && isTurnCCW)
  {
    Serial.println("!!! Turn CW and CCW commands are received at the same time !!!");
    isAlarmOn = true;
  }

  if (isAlarmOn)
  {
    digitalWrite(LED_ALARM, HIGH);
  }
  else
  {
    digitalWrite(LED_ALARM, LOW);
  }

  if (isAlarmOn)
    return;

  if (isMoveLeft && !isLeftMax)
  {
    moveLeft();
  }
  else if (isMoveRight && !isRightMax)
  {
    moveRight();
  }
  else
  {
    stopMoving();
  }

  if (isTurnCW)
  {
    turnCW();
  }
  else if (isTurnCCW)
  {
    turnCCW();
  }
  else
  {
    stopTurning();
  }

  // printCore();

  delay(50);
}