#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

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
    typedef void (*ObserverFunction)(Key, String &);

    /**
     * Registers a function to be called when the given setting is updated.
     *
     * @param      key       The setting to observe
     * @param[in]  observer  The function to be called. See above.
     */
    void registerObserver(Key key, ObserverFunction observer);

    /**
     * Gets a setting.
     *
     * @param[in]  key    The key to get
     *
     * @return     A const reference to the key's value
     */
    const String & operator[](Key key) const;

    /**
     * Gets a setting.
     *
     * @param[in]  key    The key to get
     *
     * @return     A const reference to the key's value
     */
    const String & get(Key key) const;

    /**
     * Sets a setting.
     *
     * @param[in]  key   The key to set
     *
     * @return     A reference to the key's value
     */
    String & operator[](Key key);

    /**
     * Sets a setting.
     *
     * @param[in]  key   The key to set
     * @param[in]  key   The value to change it to
     */
    void set(Key key, String & value);

private:

    String generateSettingsPage();
    void save();

    MDNSResponder mdns;
    ESP8266WebServer server;

    String store[NUM_KEYS];
    ObserverFunction observers[NUM_KEYS];
};

extern SettingsClass Settings;
