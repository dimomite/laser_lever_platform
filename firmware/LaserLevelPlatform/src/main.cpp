#include <Arduino.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "remoteserver.hpp"
#include "motorcontol.h"

#define DEBUG_INPUT (false)
#define DEBUG_OUT_ACTIONS (false)

#define LED_ALARM (16)

#define IN_LEFT_MAX (25)
#define IN_RIGHT_MAX (26)

#define IN_MOVE_LEFT (14)
#define IN_MOVE_RIGHT (12)
#define IN_TURN_CW (13)
#define IN_TURN_CCW (15)

#define OUT_DIR_1 (0)
#define OUT_STEP_1 (2)
#define OUT_DIR_2 (5)
#define OUT_STEP_2 (4)

static constexpr touch_value_t threshold = 30;

static Adafruit_SSD1306 display(128, 64, &Wire, -1);

SemaphoreHandle_t mutex = nullptr;
static StaticSemaphore_t mutexBuffer;
PlatformStatus platformStatus = {
    .linear = LinearMovementState::UNDEFINED,
    .rotation = RotationMovementState::UNDEFINED,
    .error = PlatformError::NONE,
};

static void startServerTask()
{
  xTaskCreate(
      remoteContolServerTaks, // task function
      "server_task",          // task name
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
    display.fillCircle(28, 16, 8, WHITE);
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

  pinMode(OUT_STEP_1, OUTPUT);
  pinMode(OUT_DIR_1, OUTPUT);
  pinMode(OUT_STEP_2, OUTPUT);
  pinMode(OUT_DIR_2, OUTPUT);

  digitalWrite(OUT_DIR_1, LOW);
  digitalWrite(OUT_STEP_1, LOW);

  digitalWrite(OUT_DIR_2, LOW);
  digitalWrite(OUT_STEP_2, LOW);

  pinMode(IN_LEFT_MAX, INPUT_PULLUP);
  pinMode(IN_RIGHT_MAX, INPUT_PULLUP);

  // prepare mutex for platformStatus
  mutex = xSemaphoreCreateMutexStatic(&mutexBuffer);
  if (mutex)
    Serial.println("Mutex created.");
  else
    Serial.println("Could not create mutex.");

  startServerTask();
}

static bool isAlarmOn = false;

static void moveLeft()
{
  digitalWrite(OUT_DIR_1, HIGH);
  digitalWrite(OUT_STEP_1, HIGH);
  delay(1);
  digitalWrite(OUT_STEP_1, LOW);
#if (DEBUG_OUT_ACTIONS)
  Serial.println("Moving left...");
#endif
}

static void moveRight()
{
  digitalWrite(OUT_DIR_1, LOW);
  digitalWrite(OUT_STEP_1, HIGH);
  delay(1);
  digitalWrite(OUT_STEP_1, LOW);
#if (DEBUG_OUT_ACTIONS)
  Serial.println("Moving right...");
#endif
}

static void stopMoving() {}

static void turnCW()
{
  digitalWrite(OUT_DIR_2, HIGH);
  digitalWrite(OUT_STEP_2, HIGH);
  delay(1);
  digitalWrite(OUT_STEP_2, LOW);
#if (DEBUG_OUT_ACTIONS)
  Serial.println("Turning CW...");
#endif
}

static void turnCCW()
{
  digitalWrite(OUT_DIR_2, LOW);
  digitalWrite(OUT_STEP_2, HIGH);
  delay(1);
  digitalWrite(OUT_STEP_2, LOW);
#if (DEBUG_OUT_ACTIONS)
  Serial.println("Turning CCW...");
#endif
}

static void stopTurning() {}

void loop()
{
  auto isLeftMax = digitalRead(IN_LEFT_MAX) == HIGH;
  auto isRightMax = digitalRead(IN_RIGHT_MAX) == HIGH;
  auto isMoveLeft = touchRead(IN_MOVE_LEFT) < threshold;
  auto isMoveRight = touchRead(IN_MOVE_RIGHT) < threshold;
  auto isTurnCW = touchRead(IN_TURN_CW) < threshold;
  auto isTurnCCW = touchRead(IN_TURN_CCW) < threshold;

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

  LinearMovementState linear = LinearMovementState::UNDEFINED;
  RotationMovementState rotation = RotationMovementState::UNDEFINED;
  PlatformError error = PlatformError::NONE;

  isAlarmOn = false;
  if (isLeftMax && isRightMax)
  {
    error = PlatformError::BOTH_END_STOPS_ACTIVE;
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

  if (isAlarmOn) // TODO this one will block status update
    return;

  if (!isMoveLeft && !isMoveRight)
  {
    linear = LinearMovementState::STOPPED;
  }

  if (!isLeftMax)
  {
    if (isMoveLeft)
    {
      linear = LinearMovementState::MOVING_LEFT;
    }
  }
  else
  {
    if (!isMoveRight)
    {
      linear = LinearMovementState::REACHED_MAX_LEFT;
    }
  }

  if (!isRightMax)
  {
    if (isMoveRight)
    {
      linear = LinearMovementState::MOVING_RIGHT;
    }
  }
  else
  {
    if (!isMoveLeft)
    {
      linear = LinearMovementState::REACHED_MAX_RIGHT;
    }
  }

  switch (linear)
  {
  case LinearMovementState::MOVING_LEFT:
    moveLeft();
    break;
  case LinearMovementState::MOVING_RIGHT:
    moveRight();
    break;
  case LinearMovementState::STOPPED:
  case LinearMovementState::REACHED_MAX_LEFT:
  case LinearMovementState::REACHED_MAX_RIGHT:
  case LinearMovementState::UNDEFINED:
    stopMoving();
    break;
  }

  if (isTurnCW)
  {
    rotation = RotationMovementState::TURNING_CW;
    turnCW();
  }
  else if (isTurnCCW)
  {
    rotation = RotationMovementState::TURNING_CCW;
    turnCCW();
  }
  else
  {
    rotation = RotationMovementState::STOPPED;
    stopTurning();
  }

  // printCore();

  if (mutex)
  {
    if (xSemaphoreTake(mutex, 10 / portTICK_PERIOD_MS) == pdTRUE)
    {
      platformStatus.linear = linear;
      platformStatus.rotation = rotation;
      platformStatus.error = error;
      xSemaphoreGive(mutex);
    }
    else
    {
      Serial.println("Could not take mutex to update platform status.");
    }
  }

  delay(50);
}
