#include "Settings.h"
#include "Geolocation.h"
#include "timezones.h"
#include <EEPROM.h>
#include "FS.h"

#define SETTINGS_DOMAIN     "clock" // Without the .local ending

#define MAGIC_ADDRESS       511
#define MAGIC_NUMBER        0x42

#define RED                 0xff0000
#define GREEN               0x00ff00
#define BLUE                0x0000ff

SettingsClass::SettingsClass()
    : server(80) { }

void SettingsClass::begin() {

    // Initialize the store to default values
    memset(&store, 0, sizeof(store));
    store[SET_HOUR_COLOR] = BLUE;
    store[SET_MINUTE_COLOR] = GREEN;
    store[SET_SECOND_COLOR] = RED;
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

    // Ignore unchanged values
    if (value != store[key]) {

        store[key] = value;
        
        EEPROM.put(key * sizeof(int), value);
        EEPROM.commit();

        // Notify the observers
        for (auto& it : observers[key]) {
            (it)(key, value);
        }
    }
}

String SettingsClass::buildJSON() {

    String json = "{";
    
    json.concat("\"timezone\":" + String(get(SET_TIMEZONE)));
    json.concat(",\"brightness\":" + String(get(SET_BRIGHTNESS)));
    json.concat(",\"hour_color\":" + String(get(SET_HOUR_COLOR)));
    json.concat(",\"minute_color\":" + String(get(SET_MINUTE_COLOR)));
    json.concat(",\"second_color\":" + String(get(SET_SECOND_COLOR)));

    json.concat("}");
    return json;
}

bool SettingsClass::save() {

    bool isSuccess = true;

    if (server.hasArg("tz")) {

        int tz = server.arg("tz").toInt();
        set(SET_TIMEZONE, tz);
    }

    if (server.hasArg("brightness")) {

        int brightness = server.arg("brightness").toInt();

        // Make sure input is between 1 and 100
        if (brightness >= 0 && brightness <= 100) {
            set(SET_BRIGHTNESS, brightness);
        } else {
            isSuccess = false;
        }
    }

    if (server.hasArg("hour_color")) {

        String colorCode = server.arg("hour_color");

        // Convert the hex string to an int. Offset to remove the preceding #
        int color = (int)strtol( &colorCode[1], NULL, 16);
        if (color <= 0xffffff && color >= 0x000000) {
            set(SET_HOUR_COLOR, color);
        } else {
            isSuccess = false;
        }
    }

    if (server.hasArg("minute_color")) {

        String colorCode = server.arg("minute_color");

        // Convert the hex string to an int. Offset to remove the preceding #
        int color = (int)strtol( &colorCode[1], NULL, 16);
        if (color <= 0xffffff && color >= 0x000000) {
            set(SET_MINUTE_COLOR, color);
        } else {
            isSuccess = false;
        }
    }

    if (server.hasArg("second_color")) {

        String colorCode = server.arg("second_color");

        // Convert the hex string to an int. Offset to remove the preceding #
        int color = (int)strtol( &colorCode[1], NULL, 16);
        if (color <= 0xffffff && color >= 0x000000) {
            set(SET_SECOND_COLOR, color);
        } else {
            isSuccess = false;
        }
    }

    return isSuccess;
}

SettingsClass Settings;
