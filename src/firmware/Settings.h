#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <vector>

/**
 * Represents the various possible settings keys.
 */
enum Key {
    SELECTED_TIMEZONE,
    NUM_KEYS
};

/**
 * Manages the system settings and provides an interface for changing them.
 */
class SettingsClass {
    
public:
    
    /**
     * Constructs a new SettingsClass.
     */
    SettingsClass();

    /**
     * Initializes the web server.
     */
    void begin();

    /**
     * Handles incoming connections.
     * 
     * Call this inside loop()
     */
    void handleClients();

    /**
     * Function signature for an observer function.
     */
    typedef void (*ObserverFunction)(Key, int);

    /**
     * Registers a function to be called when the given key's value is updated.
     *
     * @param      key       The setting to observe
     * @param[in]  observer  The function to be called. See above.
     */
    void registerObserver(Key key, ObserverFunction observer);

    /**
     * Unregisters all instances of an observer from a key.
     *
     * @param      key       The setting being observed
     * @param[in]  observer  The function to unregister.
     */
    void unregisterObserver(Key key, ObserverFunction observer);

    /**
     * Gets a setting.
     *
     * @param[in]  key    The key to get
     *
     * @return     The key's value
     */
    int get(Key key);

    /**
     * Sets a setting.
     *
     * @param[in]  key   The key to set
     * @param[in]  key   The value to change it to
     */
    void set(Key key, int value);

private:

    void initializeEEPROM();
    String buildJSON();
    bool save();

    MDNSResponder mdns;
    ESP8266WebServer server;

    int store[NUM_KEYS];
    std::vector<ObserverFunction> observers[NUM_KEYS];
};

extern SettingsClass Settings;
