#include "WebServer.h"
#include "DataStore.h"
#include "FS.h"

WebServerClass::WebServerClass()
    : server(80) { }

void WebServerClass::begin(const char* domain) {

    mdns.begin(domain, WiFi.localIP());

    SPIFFS.begin();

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

    server.on("/debug/resetSettings", [this]() {
        handleResetSettings();
    });

    server.begin();
    Serial.println("Web server is running at http://" + String(domain) + ".local");
}

void WebServerClass::handleClients() {

    server.handleClient();
}

bool WebServerClass::parse(String key, String value) {

    DSKey newKey;

    if (key.equals("tz")) {
        newKey = DS_TIMEZONE;
    } else if (key.equals("brightness")) {
        newKey = DS_BRIGHTNESS;
    } else if (key.equals("hour_color")) {
        newKey = DS_HOUR_COLOR;
    } else if (key.equals("minute_color")) {
        newKey = DS_MINUTE_COLOR;
    } else if (key.equals("second_color")) {
        newKey = DS_SECOND_COLOR;
    } else if (key.equals("animation")) {
        newKey = DS_CLOCK_ANIMATION;
    } else if (key.equals("night_mode_enable")) {
        newKey = DS_NIGHT_MODE_ENABLE;
    } else if (key.equals("night_mode_brightness")) {
        newKey = DS_NIGHT_MODE_BRIGHTNESS;
    } else if (key.equals("gamma")) {
        newKey = DS_GAMMA;
    } else {
        return false;
    }

    int newVal;
    float f;

    switch(newKey) {

        case DS_TIMEZONE:
        case DS_BRIGHTNESS:
        case DS_CLOCK_ANIMATION:
        case DS_NIGHT_MODE_ENABLE:
        case DS_NIGHT_MODE_BRIGHTNESS:
            newVal = value.toInt();
            break;

        case DS_HOUR_COLOR:
        case DS_MINUTE_COLOR:
        case DS_SECOND_COLOR:
            // Convert the hex string to an int. Offset to remove the preceding # symbol.
            newVal = (int)strtol(&value[1], NULL, 16);
            break;

        case DS_GAMMA:
            // Force the 32-bit float into an int type for storage
            f = value.toFloat();
            newVal = *reinterpret_cast<int*>(&f);
            break;

        default:
            newVal = -1;
            break;
    }

    Serial.println(key + "," + value + " -> " +
                   String(newKey) + "," + String(newVal));

    return DataStore.set(newKey, newVal);
}

void WebServerClass::handleSettingsSave() {

    // Keeps track whether any of the settings failed
    bool isSuccess = true;

    for (int i = 0; i < server.args(); i++) {
        isSuccess = isSuccess && parse(server.argName(i), server.arg(i));
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
    json.concat(",\"animation\":" + String(DataStore.get(DS_CLOCK_ANIMATION)));

    json.concat(",\"night_mode\":{");
        json.concat("\"enabled\":" + String(DataStore.get(DS_NIGHT_MODE_ENABLE)));
        json.concat(",\"brightness\":" + String(DataStore.get(DS_NIGHT_MODE_BRIGHTNESS)));
    json.concat("}");

    int i = DataStore.get(DS_GAMMA);
    float gamma = *reinterpret_cast<float*>(&i);
    json.concat(",\"gamma\":" + String(gamma));

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

void WebServerClass::handleResetSettings() {

    DataStore.resetSettings();
    server.send(200, "text/plain", "OK");
    Serial.println("Settings reset");
}

WebServerClass WebServer;
