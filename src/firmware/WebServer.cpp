#include "WebServer.h"
#include "DataStore.h"
#include "FS.h"

#define WEBSERVER_DOMAIN                "clock" // Without the .local ending

WebServerClass::WebServerClass()
    : server(80) { }

void WebServerClass::begin() {

    mdns.begin(WEBSERVER_DOMAIN, WiFi.localIP());

    SPIFFS.begin();

    server.serveStatic("/dummy.txt", SPIFFS, "/dummy.txt");
    server.serveStatic("/settings/dummy2.txt", SPIFFS, "/settings/dummy2.txt");
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
    Serial.println("Web server is running at http://" WEBSERVER_DOMAIN ".local");
}

// void WebServerClass::serveStatic(const char* uri, fs::FS& fs, const char* path, const char* cache_header) {

//     server.on(uri, [this, fs, path]() {

//         WiFiClient client = server.client();

//         File f = SPIFFS.open(path, "r");
//         int size = f.size();

//         client.println("HTTP/1.1 200 OK");
//         client.println("Content-Type: text/plain");
//         client.println("Content-Length: " + String(size));
//         client.println("Connection: close");
//         client.println("Access-Control-Allow-Origin: *");
//         client.println();

//         f.setTimeout(50);

//         while (f.available() || (f.position() < (size - 1))) {
//             client.println(f.readStringUntil('\n'));
//         }

//         f.close();
//     });
// }

void WebServerClass::handleClients() {

    server.handleClient();
}

void WebServerClass::handleSettingsSave() {

    // Keeps track whether any of the settings failed
    bool isSuccess = true;

    if (server.hasArg("tz")) {

        int tz = server.arg("tz").toInt();
        isSuccess = isSuccess && DataStore.set(DS_TIMEZONE, tz);
    }

    if (server.hasArg("brightness")) {

        int brightness = server.arg("brightness").toInt();
        isSuccess = isSuccess && DataStore.set(DS_BRIGHTNESS, brightness);
    }

    if (server.hasArg("hour_color")) {

        String colorCode = server.arg("hour_color");

        // Convert the hex string to an int. Offset to remove the preceding #
        int color = (int)strtol( &colorCode[1], NULL, 16);
        isSuccess = isSuccess && DataStore.set(DS_HOUR_COLOR, color);
    }

    if (server.hasArg("minute_color")) {

        String colorCode = server.arg("minute_color");

        // Convert the hex string to an int. Offset to remove the preceding #
        int color = (int)strtol( &colorCode[1], NULL, 16);
        isSuccess = isSuccess && DataStore.set(DS_MINUTE_COLOR, color);
    }

    if (server.hasArg("second_color")) {

        String colorCode = server.arg("second_color");

        // Convert the hex string to an int. Offset to remove the preceding #
        int color = (int)strtol( &colorCode[1], NULL, 16);
        isSuccess = isSuccess && DataStore.set(DS_SECOND_COLOR, color);
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
    json.concat(",\"hour_color\":" + String(DataStore.get(DS_HOUR_COLOR)));
    json.concat(",\"minute_color\":" + String(DataStore.get(DS_MINUTE_COLOR)));
    json.concat(",\"second_color\":" + String(DataStore.get(DS_SECOND_COLOR)));

    json.concat("}");
    
    server.send(200, "application/json", json);
}

void WebServerClass::handleReset() {

    server.send(200, "text/plain", "OK");
    Serial.println("System is going down for reset now!");
    ESP.restart();
}

WebServerClass WebServer;
