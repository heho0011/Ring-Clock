#include "config.h"
#include "Geolocation.h"
#include "InternetTime.h"
#include "Settings.h"
#include <Time.h>

#define TIMEOUT                     2000 // ms
#define MAX_CONNECTION_ATTEMPTS     3
#define LOCATION_SERVICE_HOST       "ipinfo.io"
#define TIMEZONE_SERVICE_HOST       "api.timezonedb.com"

extern String timezones[];

void onTimezoneUpdate(Key key, int value) {

    Serial.println("Timezone updated!");
    // time_t currentTime = InternetTime.getTime();
    // setTime(currentTime);
}

void GeolocationClass::begin() {

    Settings.registerObserver(SELECTED_TIMEZONE, &onTimezoneUpdate);
    updatePosition();
}

int GeolocationClass::getTimezoneOffset() {

    // Update timezone every request so that DST is handled
    if (!updateTimezone()) {
        return 0;
    }

    return timezoneOffset;
}

float GeolocationClass::getLatitude() {

    if (!hasBeenLocated) {
        updatePosition();
    }

    return latitude;
}

float GeolocationClass::getLongitude() {

    if (!hasBeenLocated) {
        updatePosition();
    }

    return longitude;
}

String GeolocationClass::getDetectedTimezone() {

    if (detectedTimezone.equals("")) {
        updateTimezone();
    }

    return detectedTimezone;
}

bool GeolocationClass::updatePosition() {

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

bool GeolocationClass::updateTimezone() {

    String url = "/?format=json";

    int selectedTimezone = Settings.get(SELECTED_TIMEZONE);
    bool autoDetectTimezone = (selectedTimezone == 0);

    if (autoDetectTimezone) {

        if (!hasBeenLocated) {
            if (!updatePosition()) {
                return false;
            }
        }

        url += "&lat=" + String(getLatitude());
        url += "&lng=" + String(getLongitude());

    } else {
        url += "&zone=" + timezones[selectedTimezone];
    }


    url += "&key=" + String(TIMEZONE_SERVICE_KEY);

    if (!httpGet(TIMEZONE_SERVICE_HOST, url)) {
        return false;
    }

    // Wait for response to arrive
    while (!wifi.available()) { }

    // Read the response
    
    if (autoDetectTimezone) {
        if (!wifi.find("\"zoneName\":\"")) {
            Serial.println("Could not parse timezone name");
        } else {
            detectedTimezone = wifi.readStringUntil('\"');
        }
    }

    if (!wifi.find("\"gmtOffset\":\"")) {
        Serial.println("Could not parse timezone offset");
        return false;
    } else {
        timezoneOffset = wifi.readStringUntil('\"').toFloat();
    }

    Serial.print("Detected timezone: ");
    Serial.println(timezoneOffset/3600);

    return true;
}

bool GeolocationClass::httpGet(const String hostname, const String url) {

    long lastRequestTime = millis();
    int numberOfTimeouts = 0;

    // Wait for a response
    while (!wifi.connect(hostname.c_str(), 80)) {

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
