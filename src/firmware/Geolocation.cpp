#include "Geolocation.h"
#include "config.h"

#define HTTP_PORT                   80
#define TIMEOUT                     2000 // ms
#define MAX_CONNECTION_ATTEMPTS     3

#define LOCATION_SERVICE_HOST       "ipinfo.io"

#define TIMEZONE_SERVICE_HOST       "api.timezonedb.com"


void GeolocationClass::begin() {

    getCurrentPosition();
}

int GeolocationClass::getTimezoneOffset() {

    // Update timezone every request so that DST is handled
    if (!getCurrentTimezone()) {
        return 0;
    }

    return timezoneOffset;
}

float GeolocationClass::getLatitude() {

    if (!hasBeenLocated) {
        getCurrentPosition();
    }

    return latitude;
}

float GeolocationClass::getLongitude() {

    if (!hasBeenLocated) {
        getCurrentPosition();
    }

    return longitude;
}

bool GeolocationClass::getCurrentPosition() {

    if (!httpGet(LOCATION_SERVICE_HOST, "/loc")) {
        return false;
    }

    // Wait for response to arrive
    while (!wifi.available()) { }

    // Parse the response
    wifi.find("\r\n\r\n");
    String message = wifi.readStringUntil(',');
    latitude = message.toFloat();

    message = wifi.readStringUntil('\r');
    longitude = message.toFloat();

    Serial.print("Detected location: ");
    Serial.print(latitude);
    Serial.print(", ");
    Serial.println(longitude);

    hasBeenLocated = true;

    return true;
}

bool GeolocationClass::getCurrentTimezone() {

    if (!hasBeenLocated) {
        if (!getCurrentPosition()) {
            return false;
        }
    }

    String url = "/?format=json";
    url += "&lat=" + String(getLatitude());
    url += "&lng=" + String(getLongitude());
    url += "&key=" + String(TIMEZONE_SERVICE_KEY);

    if (!httpGet(TIMEZONE_SERVICE_HOST, url)) {
        return false;
    }

    // Wait for response to arrive
    while (!wifi.available()) { }

    // Read the response
    if (!wifi.find("\"gmtOffset\":\"")) {
        Serial.println("Could not parse timezone");
    }

    String message = wifi.readStringUntil('\"');

    timezoneOffset = message.toFloat();

    Serial.print("Detected timezone: ");
    Serial.println(timezoneOffset/3600);

    return true;
}

bool GeolocationClass::httpGet(const String hostname, const String url) {

    long lastRequestTime = millis();
    int numberOfTimeouts = 0;

    // Wait for a response
    while (!wifi.connect(hostname.c_str(), HTTP_PORT)) {

        if (millis() - lastRequestTime > (TIMEOUT << numberOfTimeouts)) {
            Serial.println("Connection to '" + hostname + "' timed out.");
            lastRequestTime = millis();
            numberOfTimeouts++;
        }

        if (numberOfTimeouts >= MAX_CONNECTION_ATTEMPTS) {
            Serial.println("Error: Could not connect to '" + hostname + "'");
            return false;
        }

        yield(); // Reset the watchdog
    }

    String message = "GET " + url + " HTTP/1.1\r\n";
    message += "Host: " + hostname + "\r\n";
    message += "Connection: close\r\n";
    message += "\r\n";

    wifi.print(message);

    return true;
}

GeolocationClass Geolocation;
