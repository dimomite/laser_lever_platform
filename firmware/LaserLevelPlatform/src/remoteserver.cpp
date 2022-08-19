#include "remoteserver.hpp"

#include <Arduino.h>
#include <ESPmDNS.h>
#include <WiFi.h>

static constexpr char *ssid = "Laser level platform";
static constexpr char *password = "laserlevel15";
static constexpr char *staticUrl = "laserlevel";

static constexpr uint16_t port = 80;
static String instanceName = "Laser level platform";

static bool prepareAp();
static bool stopAp();

static void watchConnectionsCount();
static bool startMDns();
static bool startListeningServerConnections();

static WiFiServer server(port);

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
        startListeningServerConnections();
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

bool startListeningServerConnections()
{
    server.begin();

    while (true)
    {
        WiFiClient client = server.available();
        Serial.println("Got a client");
        client.stop();
        Serial.println("Client disconnected");
    }

    return true;
}

bool startMDns()
{
    if (!MDNS.begin(staticUrl)) {
        Serial.print("Could not start mDNS with URL: \"");
        Serial.print(staticUrl);
        Serial.println("\"");
        return false;
    }

    if (!MDNS.addService("_http", "_tcp", port)) { // leading underscore is not mandatory, functions adds it if not added
        Serial.print("Could not add service \"http\", protocol: \"tcp\", port: ");
        Serial.print(port);
        Serial.println(". Stopping mDNS");

        MDNS.end();
        return false;
    }
    MDNS.setInstanceName(instanceName);

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
