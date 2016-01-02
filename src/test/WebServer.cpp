#include "WebServer.h"
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
    server.send(200, "text/plain", "OK");
}

void WebServerClass::handleSettingsGet() {

    server.send(200, "application/json", "{}");
}

void WebServerClass::handleReset() {

    server.send(200, "text/plain", "OK");
    Serial.println("System is going down for reset now!");
    ESP.restart();
}

WebServerClass WebServer;
