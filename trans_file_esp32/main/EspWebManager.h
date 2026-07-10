#ifndef ESPWEBMANAGER_H
#define ESPWEBMANAGER_H

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

class EspWebManager {
public:
    EspWebManager();
    void begin();
    void loop();
    void sendToArduino(String data);

private:
    AsyncWebServer server;
    String wifiOptions;
    bool shouldConnect;
    String reqSSID;
    String reqPass;
    int connectStatus;
    String newIP;
    bool shouldTurnOffAP;
    unsigned long turnOffTime;

    void scanWiFi();
    void setupRoutes();
};

#endif