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

#define OUT_DIR_TURN (5)
#define OUT_STEP_TURN (4)
#define OUT_DIR_MOVE (0)
#define OUT_STEP_MOVE (2)

static void stopMoving();
static void stopTurning();

static constexpr touch_value_t threshold = 50;

static constexpr uint8_t rotationStepsChannel = 0;
static constexpr uint8_t movementStepsChannel = 1;
static constexpr uint32_t stepsFrequency = 200; // in Hz
static constexpr uint8_t stepsResolution = 18;
static constexpr uint32_t stepWidth = (1 << (stepsResolution - 1)); // 1/8th of a pulse

// duration of actions initiated by a button press
static constexpr int32_t fixedMoveDuration = 10;
static constexpr int32_t fixedTurnDuration = 10;

static Adafruit_SSD1306 display(128, 64, &Wire, -1);

SemaphoreHandle_t mutex = nullptr;
static StaticSemaphore_t mutexBuffer;
PlatformStatus platformStatus = {
    .linear = LinearMovementState::UNDEFINED,
    .rotation = RotationMovementState::UNDEFINED,
    .error = PlatformError::NONE,
};

static constexpr auto queueLength = 16;
static constexpr auto queueBufferSize = queueLength * sizeof(ActionCommand);
static StaticQueue_t staticCommandsQueue;
static uint8_t commandsQueueBuffer[queueBufferSize];
QueueHandle_t motorCommandsQueue;
static ActionCommand cmdReceiver;

static volatile int32_t movementCounter = 0;
static volatile int32_t rotationCounter = 0;

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

void IRAM_ATTR onPwmTimer()
{
  if (movementCounter > 0)
  {
    --movementCounter;
    if (0 == movementCounter)
    {
      stopMoving();
      Serial.println("Expired movement counter");
    }
  }

  if (rotationCounter > 0)
  {
    --rotationCounter;
    if (0 == rotationCounter)
    {
      stopTurning();
      Serial.println("Expired rotation counter");
    }
  }
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

  pinMode(OUT_STEP_TURN, OUTPUT);
  pinMode(OUT_DIR_TURN, OUTPUT);
  pinMode(OUT_STEP_MOVE, OUTPUT);
  pinMode(OUT_DIR_MOVE, OUTPUT);

  digitalWrite(OUT_DIR_TURN, LOW);
  digitalWrite(OUT_DIR_MOVE, LOW);

  pinMode(IN_LEFT_MAX, INPUT_PULLUP);
  pinMode(IN_RIGHT_MAX, INPUT_PULLUP);

  auto timer = timerBegin(0, 2, true);
  timerAlarmWrite(timer, 1000000, true);
  auto rotationSetup = ledcSetup(rotationStepsChannel, stepsFrequency, stepsResolution);
  auto linearSetup = ledcSetup(movementStepsChannel, stepsFrequency, stepsResolution);
  if ((rotationSetup != 0) && (linearSetup != 0))
  {
    Serial.print("Rotation steps setup finish with frequency: ");
    Serial.println(rotationSetup);

    Serial.print("Movement steps setup finish with frequency: ");
    Serial.println(linearSetup);

    ledcAttachPin(OUT_STEP_MOVE, movementStepsChannel);
    ledcAttachPin(OUT_STEP_TURN, rotationStepsChannel);

    timerAttachInterrupt(timer, onPwmTimer, false);
    timerAlarmEnable(timer);
  }
  else
  {
    timerEnd(timer);
    Serial.println("LEDC setup failed");
  }

  // prepare mutex for platformStatus
  mutex = xSemaphoreCreateMutexStatic(&mutexBuffer);
  if (mutex)
    Serial.println("Mutex created.");
  else
    Serial.println("Could not create mutex.");

  motorCommandsQueue = xQueueCreateStatic(queueLength, sizeof(ActionCommand), commandsQueueBuffer, &staticCommandsQueue);
  if (motorCommandsQueue)
    Serial.println("Motor commands queue created.");
  else
    Serial.println("Could not create queue for motor commands.");

  startServerTask();
} // setup()

static bool isAlarmOn = false;

static void moveLeft()
{
  digitalWrite(OUT_DIR_MOVE, HIGH);
  ledcWrite(movementStepsChannel, stepWidth);
#if (DEBUG_OUT_ACTIONS)
  Serial.println("Moving left...");
#endif
}

static void moveRight()
{
  digitalWrite(OUT_DIR_MOVE, LOW);
  ledcWrite(movementStepsChannel, stepWidth);
#if (DEBUG_OUT_ACTIONS)
  Serial.println("Moving right...");
#endif
}

void stopMoving()
{
  ledcWrite(movementStepsChannel, 0);
}

static void turnCW()
{
  digitalWrite(OUT_DIR_TURN, HIGH);
  ledcWrite(rotationStepsChannel, stepWidth);
#if (DEBUG_OUT_ACTIONS)
  Serial.println("Turning CW...");
#endif
}

static void turnCCW()
{
  digitalWrite(OUT_DIR_TURN, LOW);
  ledcWrite(rotationStepsChannel, stepWidth);
#if (DEBUG_OUT_ACTIONS)
  Serial.println("Turning CCW...");
#endif
}

void stopTurning()
{
  ledcWrite(rotationStepsChannel, 0);
}

void loop()
{
  auto isLeftMax = digitalRead(IN_LEFT_MAX) == HIGH;
  auto isRightMax = digitalRead(IN_RIGHT_MAX) == HIGH;
  auto left = touchRead(IN_MOVE_LEFT);
  auto right = touchRead(IN_MOVE_RIGHT);
  auto cw = touchRead(IN_TURN_CW);
  auto ccw = touchRead(IN_TURN_CCW);
  auto isMoveLeft = left < threshold;
  auto isMoveRight = right < threshold;
  auto isTurnCW = cw < threshold;
  auto isTurnCCW = ccw < threshold;

  // Serial.print("Left: ");
  // Serial.print(left);
  // Serial.print(", right: ");
  // Serial.print(right);
  // Serial.print(", cw: ");
  // Serial.print(cw);
  // Serial.print(", ccw: ");
  // Serial.println(ccw);

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

  if ((movementCounter > 0) && (isLeftMax || isRightMax))
  {
    // stop movement on reaching endstop
    movementCounter = 0;
    stopMoving();
    // TODO need to add proper recovery from remove device when limit is reached
  }

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

  if (!isLeftMax)
  {
    if (isMoveLeft)
    {
      linear = LinearMovementState::MOVING_LEFT;
      movementCounter = fixedMoveDuration;
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
      movementCounter = fixedMoveDuration;
    }
  }
  else
  {
    if (!isMoveLeft)
    {
      linear = LinearMovementState::REACHED_MAX_RIGHT;
    }
  }

  if (isTurnCW)
  {
    rotation = RotationMovementState::TURNING_CW;
    rotationCounter = fixedTurnDuration;
  }
  else if (isTurnCCW)
  {
    rotation = RotationMovementState::TURNING_CCW;
    rotationCounter = fixedTurnDuration;
  }

  while (uxQueueMessagesWaiting(motorCommandsQueue))
  {
    auto received = xQueueReceive(motorCommandsQueue, &cmdReceiver, 0);
    if (received)
    {
      switch (cmdReceiver.type)
      {
      case ActionCommandType::Left:
        if (!isLeftMax)
        {
          linear = LinearMovementState::MOVING_LEFT;
          movementCounter = cmdReceiver.duration;
        }
        break;
      case ActionCommandType::Right:
        if (!isRightMax)
        {
          linear = LinearMovementState::MOVING_RIGHT;
          movementCounter = cmdReceiver.duration;
        }
        break;
      case ActionCommandType::CW:
        rotation = RotationMovementState::TURNING_CW;
        rotationCounter = cmdReceiver.duration;
        break;
      case ActionCommandType::CCW:
        rotation = RotationMovementState::TURNING_CCW;
        rotationCounter = cmdReceiver.duration;
        break;
      case ActionCommandType::StopMove:
        linear = LinearMovementState::STOPPED;
        movementCounter = 0;
        break;
      case ActionCommandType::StopTurn:
        rotation = RotationMovementState::STOPPED;
        rotationCounter = 0;
        break;
      case ActionCommandType::Undefined:
        break;
      }
    }
    else
    {
      Serial.println("Has command in the buffer but could not receive");
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
    stopMoving();
    break;
  case LinearMovementState::REACHED_MAX_LEFT:
  case LinearMovementState::REACHED_MAX_RIGHT:
  case LinearMovementState::UNDEFINED:
    break;
  }

  switch (rotation)
  {
  case RotationMovementState::TURNING_CW:
    turnCW();
    break;
  case RotationMovementState::TURNING_CCW:
    turnCCW();
    break;
  case RotationMovementState::STOPPED:
    stopTurning();
    break;
  case RotationMovementState::UNDEFINED:
    break;
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
