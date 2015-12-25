#include "Settings.h"
#include "Geolocation.h"
#include "timezones.h"
#include <EEPROM.h>
#include "FS.h"

#define SETTINGS_DOMAIN     "clock" // Without the .local ending

#define MAGIC_ADDRESS       511
#define MAGIC_NUMBER        0x42

SettingsClass::SettingsClass()
    : server(80) { }

void SettingsClass::begin() {

    // Initialize the store to default values
    memset(&store, 0, sizeof(store));
    initializeEEPROM();

    mdns.begin(SETTINGS_DOMAIN, WiFi.localIP());

    SPIFFS.begin();

    server.serveStatic("/", SPIFFS, "/settings.html");
    server.serveStatic("/settings.js", SPIFFS, "/settings.js");

    server.on("/saveSettings", [this](){
        
        if (save()) {
            server.sendHeader("Location", "/?status=success");
        } else {
            server.sendHeader("Location", "/?status=error");
        }
        
        server.send(302);
    });

    server.on("/getSettings", [this](){
        
        server.send(200, "application/json", buildJSON());
    });

    server.begin();
    Serial.println("Settings web server is running at http://" SETTINGS_DOMAIN ".local");
}

void SettingsClass::initializeEEPROM() {

    EEPROM.begin(512);
    
    if (EEPROM.read(MAGIC_ADDRESS) != MAGIC_NUMBER) {

        // EEPROM has never been written
        // Set the EEPROM to the default store values
        EEPROM.put(0, store);

        // Mark it as written
        EEPROM.write(MAGIC_ADDRESS, MAGIC_NUMBER);

        EEPROM.commit();

    } else {
        // Get the saved values from EEPROM
        EEPROM.get(0, store);
    }
}

void SettingsClass::handleClients() {

    server.handleClient();
}

void SettingsClass::registerObserver(Key key, ObserverFunction observer) {

    observers[key].push_back(observer);
}

void SettingsClass::unregisterObserver(Key key, ObserverFunction observer) {

    for (auto it = observers[key].begin(); it != observers[key].end(); it++) {
        if (*it == observer) {
            observers[key].erase(it);
        }
    }
}

int SettingsClass::get(Key key) {

    return store[key];
}

void SettingsClass::set(Key key, int value) {

    store[key] = value;
    
    EEPROM.put(key * sizeof(int), value);
    EEPROM.commit();

    // Notify the observers
    for (auto& it : observers[key]) {
        (it)(key, value);
    }
}

String SettingsClass::buildJSON() {

    String json = "{";
    
    json.concat("\"timezone\":" + String(get(SELECTED_TIMEZONE)));

    json.concat("}");
    return json;
}

bool SettingsClass::save() {

    if (server.hasArg("tz")) {

        int tz = server.arg("tz").toInt();
        set(SELECTED_TIMEZONE, tz);
    }

    return true;
}

SettingsClass Settings;
