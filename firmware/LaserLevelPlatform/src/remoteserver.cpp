#include "remoteserver.hpp"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Adafruit_SSD1306.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>

#include "favicon.h"
#include "motorcontol.h"
#include "webcontrolpage.h"

static constexpr char *ssid = "Laser level platform";
static constexpr char *password = "laserlevel15";
// static constexpr char *staticUrl = "laserlevel";

static constexpr uint16_t port = 80;
static String instanceName = "Laser level platform";

static constexpr int HTTP_OK = 200;

extern SemaphoreHandle_t mutex;
extern PlatformStatus platformStatus;

extern QueueHandle_t motorCommandsQueue;
extern Adafruit_SSD1306 display;

static StaticJsonDocument<1024> doc;

static bool prepareAp();
static bool stopAp();

static void watchConnectionsCount();

static AsyncWebServer server(port);

static void printCore()
{
    // Serial.print("Running WiFi access point logic on core #");
    // Serial.println(xPortGetCoreID());
}

static void updateJsonFromPlatformStatus()
{
    if (xSemaphoreTake(mutex, 10 / portTICK_PERIOD_MS) == pdTRUE)
    {
        auto status = platformStatus; // make a local copy
        xSemaphoreGive(mutex);        // release mutex

        doc.clear();
        switch (status.linear)
        {
        case LinearMovementState::UNDEFINED:
            doc["linear"] = "undef";
            break;
        case LinearMovementState::STOPPED:
            doc["linear"] = "stopped";
            break;
        case LinearMovementState::MOVING_LEFT:
            doc["linear"] = "movingleft";
            break;
        case LinearMovementState::MOVING_RIGHT:
            doc["linear"] = "movingright";
            break;
        case LinearMovementState::REACHED_MAX_LEFT:
            doc["linear"] = "maxleft";
            break;
        case LinearMovementState::REACHED_MAX_RIGHT:
            doc["linear"] = "maxright";
            break;
        }

        switch (status.rotation)
        {
        case RotationMovementState::UNDEFINED:
            doc["rotation"] = "undef";
            break;
        case RotationMovementState::STOPPED:
            doc["rotation"] = "stopped";
            break;
        case RotationMovementState::TURNING_CW:
            doc["rotation"] = "cw";
            break;
        case RotationMovementState::TURNING_CCW:
            doc["rotation"] = "ccw";
            break;
        }

        switch (status.error)
        {
        case PlatformError::NONE:
            doc["error"] = "none";
            break;
        case PlatformError::BOTH_END_STOPS_ACTIVE:
            doc["error"] = "bothendstopsactive";
            break;
        }
    }
}

static ActionCommandType decodeCommandType(const String &cmd)
{
    if (cmd.equals("left"))
        return ActionCommandType::Left;
    if (cmd.equals("right"))
        return ActionCommandType::Right;
    if (cmd.equals("cw"))
        return ActionCommandType::CW;
    if (cmd.equals("ccw"))
        return ActionCommandType::CCW;

    return ActionCommandType::Undefined;
}

static void logRequestArguments(AsyncWebServerRequest *const request)
{
    Serial.print(request->url());
    Serial.print("\", args: ");
    auto argsCount = request->args();
    Serial.print(argsCount);
    Serial.print(" => ");
    for (auto i = 0; i < argsCount; ++i)
    {
        auto argName = request->argName(i);
        Serial.print(argName);
        Serial.print(": ");
        Serial.print(request->arg(argName));
        Serial.print(", ");
    }
    Serial.println();
}

static void handleMoveTurnMessages(AsyncWebServerRequest *request)
{
    auto dir = request->arg("dir");
    auto dist = request->arg("dist");

    Serial.print("Got move/turn command: \"");
    logRequestArguments(request);

    if (!dir.isEmpty() && !dist.isEmpty())
    {
        if (uxQueueSpacesAvailable(motorCommandsQueue))
        {
            ActionCommand cmd{
                .type = decodeCommandType(dir),
                .duration = dist.toInt(),
            };
            auto sent = xQueueSend(motorCommandsQueue, &cmd, 0);
            if (sent)
            {
                request->send(HTTP_OK);
            }
            else
            {
                Serial.println("Could not put command to queue");
                request->send(507); // Insufficient Storage
            }
        }
        else
        {
            Serial.println("No space in queue");
            request->send(507); // Insufficient Storage
        }
    }
    else
    {
        Serial.print("Bad request for move/turn command: \"");
        logRequestArguments(request);
        request->send(400); // Bad Request
    }
}

static void handleCommandRequest(AsyncWebServerRequest *request, const ActionCommandType command)
{
    if (uxQueueSpacesAvailable(motorCommandsQueue))
    {
        ActionCommand cmd{
            .type = command,
            .duration = 0,
        };
        auto sent = xQueueSend(motorCommandsQueue, &cmd, 0);
        if (sent)
        {
            request->send(HTTP_OK);
        }
        else
        {
            Serial.println("Could not put command to queue");
            request->send(507); // Insufficient Storage
        }
    }
    else
    {
        Serial.println("No space in queue");
        request->send(507); // Insufficient Storage
    }
}

void remoteContolServerTaks(void *args)
{
    if (prepareAp())
    {
        // watchConnectionsCount();

        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                  { request->send_P(HTTP_OK, "text/html", webControlPage); });

        server.on("/hello",
                  HTTP_GET,
                  [](AsyncWebServerRequest *request)
                  {
                      printCore();
                      request->send(HTTP_OK, "text/plain", "Hello from the Laser level platform. The laserest platform ever.");
                  });

        server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request)
                  { request->send_P(HTTP_OK, "image/x-icon", favicon, sizeof(favicon)); });

        server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
                    updateJsonFromPlatformStatus();
                    String buff = "";
                    serializeJson(doc, buff);
                    printCore();
                    request->send(HTTP_OK, "application/json", buff); });

        server.on("/api/move", HTTP_POST, [](AsyncWebServerRequest *request)
                  { handleMoveTurnMessages(request); });

        server.on("/api/stopmove", HTTP_POST, [](AsyncWebServerRequest *request)
                  { handleCommandRequest(request, ActionCommandType::StopMove); });

        server.on("/api/turn", HTTP_POST, [](AsyncWebServerRequest *request)
                  { handleMoveTurnMessages(request); });

        server.on("/api/stopturn", HTTP_POST, [](AsyncWebServerRequest *request)
                  { handleCommandRequest(request, ActionCommandType::StopTurn); });

        server.onNotFound([](AsyncWebServerRequest *request)
                          {
                              printCore();
                              Serial.println("Not defined for request: \"" + request->url() + "\"");
                              request->send(404, "text/plain", "The content you are looking for was not found."); });
        server.begin();
    }
    else
    {
        while (true)
        {
        }
    }

    vTaskDelete(nullptr);
}

bool prepareAp()
{
    if (!WiFi.softAP(ssid, password))
    {
        Serial.print("Could not create WiFi access point with name: \"");
        Serial.print(ssid);
        Serial.println("\"");
        return false;
    }

    Serial.println("Created WiFi access point");
    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());
    Serial.print("Host name: ");
    Serial.println(WiFi.softAPgetHostname());
    Serial.print("MAC address: ");
    Serial.println(WiFi.softAPmacAddress());
    Serial.print("SSID: \"");
    Serial.print(WiFi.softAPSSID());
    Serial.println("\"");

    display.clearDisplay();
    display.setCursor(0, 24);
    display.println(password);
    display.println(WiFi.softAPIP());
    display.display();

    return true;
}

bool stopAp()
{
    if (!WiFi.softAPdisconnect())
    {
        Serial.println("Could not stop WiFi access point");
        return false;
    }

    Serial.println("WiFi access point was stopped");
    return true;
}

void watchConnectionsCount()
{
    Serial.println("Waiting for devices connections");
    uint8_t activeConnections = 0;
    while (true)
    {
        uint8_t connections = WiFi.softAPgetStationNum();
        if (connections != activeConnections)
        {
            activeConnections = connections;
            Serial.print("Number of active connections: ");
            Serial.println(activeConnections);
            delay(1000);
        }
    }
}
