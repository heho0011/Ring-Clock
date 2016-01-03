#include "WebServer.h"
#include "DataStore.h"
#include "FS.h"

WebServerClass::WebServerClass()
    : server(80) { }

void WebServerClass::begin(const char* domain) {

    mdns.begin(domain, WiFi.localIP());

    SPIFFS.begin();

    server.serveStatic("/dummy.txt", SPIFFS, "/dummy.txt");
    server.serveStatic("/settings/", SPIFFS, "/settings/index.html", "max-age=86400");
    server.serveStatic("/settings/settings.js", SPIFFS, "/settings/settings.js", "max-age=86400");

    server.on("/", [this]() {
        server.sendHeader("Location", "/settings/");
        server.send(302);
    });

    server.on("/settings/save", [this]() {
        handleSettingsSave();
    });

    server.on("/settings/get", [this]() {
        handleSettingsGet();
    });

    server.on("/debug/reset", [this]() {
        handleReset();
    });

    server.begin();
    Serial.println("Web server is running at http://" + String(domain) + ".local");
}

void WebServerClass::handleClients() {

    server.handleClient();
}

void WebServerClass::handleSettingsSave() {

    // Keeps track whether any of the settings failed
    bool isSuccess = true;

    if (server.hasArg("tz")) {
        isSuccess = isSuccess && DataStore.set(DS_TIMEZONE, server.arg("tz"));
    }

    if (server.hasArg("brightness")) {
        isSuccess = isSuccess && DataStore.set(DS_BRIGHTNESS, server.arg("brightness"));
    }

    if (server.hasArg("hour_color")) {
        isSuccess = isSuccess && DataStore.set(DS_HOUR_COLOR, server.arg("hour_color"));
    }

    if (server.hasArg("minute_color")) {
        isSuccess = isSuccess && DataStore.set(DS_MINUTE_COLOR, server.arg("minute_color"));
    }

    if (server.hasArg("second_color")) {
        isSuccess = isSuccess && DataStore.set(DS_SECOND_COLOR, server.arg("second_color"));
    }

    if (isSuccess) {
        server.sendHeader("Location", "/settings/?status=success");
    } else {
        server.sendHeader("Location", "/settings/?status=error");
    }
    
    server.send(302);
}

void WebServerClass::handleSettingsGet() {

    String json = "{";
    
    json.concat("\"timezone\":" + String(DataStore.get(DS_TIMEZONE)));
    json.concat(",\"brightness\":" + String(DataStore.get(DS_BRIGHTNESS)));
    json.concat(",\"hour_color\":" + getColorCode(DataStore.get(DS_HOUR_COLOR)));
    json.concat(",\"minute_color\":" + getColorCode(DataStore.get(DS_MINUTE_COLOR)));
    json.concat(",\"second_color\":" + getColorCode(DataStore.get(DS_SECOND_COLOR)));

    json.concat("}");
    
    server.send(200, "application/json", json);
}

String WebServerClass::getColorCode(int value) {

    String s = String(value, HEX);
    String hexString = "\"#";

    for (int i = 6; i > s.length(); i--) {
        hexString.concat("0");
    }

    hexString.concat(s);

    hexString.concat("\"");

    return hexString;
}

void WebServerClass::handleReset() {

    server.send(200, "text/plain", "OK");
    Serial.println("System is going down for reset now!");
    ESP.restart();
}

WebServerClass WebServer;
