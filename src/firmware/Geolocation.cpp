#include "config.h"
#include "Geolocation.h"
#include "InternetTime.h"
#include "DataStore.h"
#include "timezones.h"
#include <Time.h>

#define TIMEOUT                     2000 // ms
#define MAX_CONNECTION_ATTEMPTS     3
#define LOCATION_SERVICE_HOST       "ipinfo.io"
#define TIMEZONE_SERVICE_HOST       "api.timezonedb.com"

extern String timezones[];

void onTimezoneUpdate(DSKey key, int value) {

    Serial.println("Timezone updated!");
    time_t currentTime = InternetTime.getTime();
    setTime(currentTime);
}

bool timezoneValidator(DSKey key, int value) {

    // Make sure the value is within the array bounds
    return (value < NUM_TIMEZONES && value >= 0);
}

void GeolocationClass::begin() {

    DataStore.registerObserver(DS_TIMEZONE, &onTimezoneUpdate);
    DataStore.registerValidator(DS_TIMEZONE, &timezoneValidator);
    updatePosition();
}

int GeolocationClass::getTimezoneOffset() {

    // Update timezone every request so that DST is handled
    if (!updateTimezone()) {

        timezoneOffset = DataStore.get(DS_LAST_TIMEZONE_OFFSET);

        Serial.print("Using previously detected timezone: ");
        Serial.println(timezoneOffset/3600);
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

    int selectedTimezone = DataStore.get(DS_TIMEZONE);
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
    time_t requestTime = millis();
    while (!wifi.available() &&
        (millis() - requestTime) < TIMEOUT) { }

    // Handle timeouts
    if (millis() - requestTime >= TIMEOUT) {
        Serial.println("Request timed out.");
        return false;
    }

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
    
    DataStore.set(DS_LAST_TIMEZONE_OFFSET, timezoneOffset);

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
