#include "Settings.h"
#include "SettingsPage.h"
#include "Geolocation.h"

SettingsClass::SettingsClass()
    : server(80) {
    }

void SettingsClass::begin() {

    if (mdns.begin("clock", WiFi.localIP())) {
        Serial.println("MDNS responder started");
    }

    server.on("/", [this](){
        server.send(200, "text/html", generateSettingsPage());
    });

    server.on("/save", [this](){
        save();
        server.send(200, "text/html", generateSettingsPage());
    });

    server.begin();
    Serial.println("HTTP server started");
}

void SettingsClass::handleClients() {
    server.handleClient();
}

void SettingsClass::registerObserver(Key key, ObserverFunction observer) {

    observers[key] = observer;
}

const String & SettingsClass::operator[](Key key) const {

    return store[key];
}

const String & SettingsClass::get(Key key) const {

    return operator[](key);
}

String & SettingsClass::operator[](Key key) {

    // Notify the observer, if any
    ObserverFunction observer = observers[key];
    if (observer) {
        observer(key, store[key]);
    }

    return store[key];
}

void SettingsClass::set(Key key, String & value) {

    operator[](key) = value;
}

String SettingsClass::generateSettingsPage() {
    
    String page = settingsPage;
    String options = "<option value='auto'>Auto-detect</option>";
    page.replace("$1", options);
    return page;
}

void SettingsClass::save() {

    if (server.hasArg("tz")) {

        String tz = server.arg("tz");
        set(SELECTED_TIMEZONE, tz);
    }
}

SettingsClass Settings;
