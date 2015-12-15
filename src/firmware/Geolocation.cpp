#include "Geolocation.h"
#include "config.h"

#define HTTP_PORT                   80

#define LOCATION_SERVICE_HOST       "ipinfo.io"

#define TIMEZONE_SERVICE_HOST       "api.timezonedb.com"


void GeolocationClass::begin() {

    locate();
}

bool GeolocationClass::locate() {

    if (!getCurrentPosition()) {
        return false;
    }

    // if (!getCurrentTimezone()) {
    //     return false;
    // }

    return true;
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

    return true;
}

bool GeolocationClass::getCurrentTimezone() {

    String url = "/?format=json";
    url += "&lat=" + String(latitude);
    url += "&lng=" + String(longitude);
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

    if (!wifi.connect(hostname.c_str(), HTTP_PORT)) {
        Serial.println("Error: Could not connect to " + hostname);
        return false;
    }

    String message = "GET " + url + " HTTP/1.1\r\n";
    message += "Host: " + hostname + "\r\n";
    message += "Connection: close\r\n";
    message += "\r\n";

    wifi.print(message);

    return true;
}

int GeolocationClass::getTimezoneOffset() {

    // Update timezone every request so that DST is handled
    if (!getCurrentTimezone()) {
        return 0;
    }

    return timezoneOffset;
}

float GeolocationClass::getLatitude() {
    return latitude;
}

float GeolocationClass::getLongitude() {
    return longitude;
}

GeolocationClass Geolocation;
