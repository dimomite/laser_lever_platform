#include "remoteserver.hpp"

#include <Arduino.h>
// #include <ESPmDNS.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

#include "favicon.h"

static constexpr char *ssid = "Laser level platform";
static constexpr char *password = "laserlevel15";
// static constexpr char *staticUrl = "laserlevel";

static constexpr uint16_t port = 80;
static String instanceName = "Laser level platform";

static constexpr int HTTP_OK = 200;

static bool prepareAp();
static bool stopAp();

static void watchConnectionsCount();
static bool startMDns();
static bool startListeningServerConnections();

static AsyncWebServer server(port);
// static WiFiServer server(port);

static void printCore()
{
    Serial.print("Running WiFi access point logic on core #");
    Serial.println(xPortGetCoreID());
}

void remoteContolServerTaks(void *args)
{
    if (prepareAp())
    {
        // watchConnectionsCount();
        // startMDns();

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
                      printCore();
                      request->send(HTTP_OK, "application/json", "{\"error\":\"none\",\"linear\":\"stopped\", \"rotation\":\"ccw\"}"); });
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

// bool startListeningServerConnections()
// {
//     server.begin();

//     while (true)
//     {
//         WiFiClient client = server.available();
//         Serial.println("Got a client");
//         client.stop();
//         Serial.println("Client disconnected");
//     }

//     return true;
// }

// bool startMDns()
// {
//     if (!MDNS.begin(staticUrl))
//     {
//         Serial.print("Could not start mDNS with URL: \"");
//         Serial.print(staticUrl);
//         Serial.println("\"");
//         return false;
//     }

//     if (!MDNS.addService("_http", "_tcp", port))
//     { // leading underscore is not mandatory, functions adds it if not added
//         Serial.print("Could not add service \"http\", protocol: \"tcp\", port: ");
//         Serial.print(port);
//         Serial.println(". Stopping mDNS");

//         MDNS.end();
//         return false;
//     }
//     MDNS.setInstanceName(instanceName);

//     return true;
// }

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
