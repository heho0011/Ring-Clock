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
#define SUNRISE_SUNSET_SERVICE_HOST "api.sunrise-sunset.org"

extern const String timezones[];

bool timezoneValidator(DSKey key, int value) {

    // Make sure the value is within the array bounds
    return (value < NUM_TIMEZONES && value >= 0);
}

void GeolocationClass::begin() {

    DataStore.registerValidator(DS_TIMEZONE, &timezoneValidator);
    updatePosition();
}

int GeolocationClass::getTimezoneOffset(bool force) {

    // Update timezone every request so that DST is handled
    if (force && !updateTimezone()) {

        timezoneOffset = DataStore.get(DS_LAST_TIMEZONE_OFFSET);

        Serial.print("Using previously detected timezone: ");
        Serial.println(timezoneOffset/3600);
    }

    return timezoneOffset;
}

float GeolocationClass::getLatitude() {

    updatePosition();
    return latitude;
}

float GeolocationClass::getLongitude() {

    updatePosition();
    return longitude;
}

time_t GeolocationClass::getSunriseTime() {

    updateSunriseSunset();
    return sunriseTime;
}

time_t GeolocationClass::getSunsetTime() {

    updateSunriseSunset();
    return sunsetTime;
}

String GeolocationClass::getDetectedTimezone() {

    if (detectedTimezone.equals("")) {
        updateTimezone();
    }

    return detectedTimezone;
}

time_t GeolocationClass::extractTime(String utc) {

    TimeElements tm;

    // Example input string: 2016-01-23T23:15:54+00:00
    // Ugly substring hacking because sscanf is not implemented on ESP8266
    tm.Year = CalendarYrToTm(utc.substring(0, 4).toInt());
    tm.Month = utc.substring(5, 7).toInt();
    tm.Day = utc.substring(8, 10).toInt();
    tm.Hour = utc.substring(11, 13).toInt();
    tm.Minute = utc.substring(14, 16).toInt();
    tm.Second = utc.substring(17, 19).toInt();

    return makeTime(tm) + getTimezoneOffset();
}

bool GeolocationClass::updatePosition() {

    // Only update if necessary
    static bool hasBeenLocated = false;
    if (hasBeenLocated) {
        return true;
    }

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

bool GeolocationClass::updateSunriseSunset() {

    // Only update if necessary
    static time_t lastChecked = 0;
    if (now() - lastChecked < SECS_PER_DAY) {
        return true;
    }

    String url = "/json?formatted=0";
    time_t t = now();
    url += "&date=" + String(year(t)) + "-" + String(month(t)) + "-" + String(day(t));
    url += "&lat=" + String(getLatitude());
    url += "&lng=" + String(getLongitude());
    if (!httpGet(SUNRISE_SUNSET_SERVICE_HOST, url)) {
        return false;
    }

    // Wait for response to arrive
    while (!wifi.available()) { }

    // Parse the response
    if (!wifi.find("\"sunrise\":\"")) {
        Serial.println("Could not parse sunrise");
        return false;
    } else {
        sunriseTime = extractTime(wifi.readStringUntil('\"'));
    }

    if (!wifi.find("\"sunset\":\"")) {
        Serial.println("Could not parse sunset");
        return false;
    } else {
        sunsetTime = extractTime(wifi.readStringUntil('\"'));
    }

    Serial.print("Detected sunrise: ");
    Serial.println(sunriseTime);
    Serial.print("Detected sunset: ");
    Serial.println(sunsetTime);

    lastChecked = now();

    return true;
}

bool GeolocationClass::updateTimezone() {

    String url = "/?format=json";

    int selectedTimezone = DataStore.get(DS_TIMEZONE);
    bool autoDetectTimezone = (selectedTimezone == 0);

    if (autoDetectTimezone) {

        if (!updatePosition()) {
            return false;
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
