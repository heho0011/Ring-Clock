#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

/**
 * Manages the web server.
 */
class WebServerClass {
    
public:
    
    /**
     * Constructs a new WebServerClass.
     */
    WebServerClass();

    /**
     * Initializes the store and web server.
     */
    void begin();

    /**
     * Handles incoming connections.
     * 
     * @note     Call this inside loop()
     */
    void handleClients();

private:

    void handleSettingsSave();
    void handleSettingsGet();
    void handleReset();
    // void serveStatic(const char* uri, fs::FS& fs, const char* path, const char* cache_header = NULL);


    MDNSResponder mdns;
    ESP8266WebServer server;

};

extern WebServerClass WebServer;
