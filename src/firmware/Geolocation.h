#pragma once

#include <ESP8266WiFi.h>

/**
 * A class to get the current position and time zone.
 */
class GeolocationClass {

public:

    /**
     * Locates the user.
     */
    void begin();

    /**
     * Gets the most recently determined latitude.
     *
     * @return     the latitude
     */
    float getLatitude();

    /**
     * Gets the most recently determined longitude.
     *
     * @return     the latitude
     */
    float getLongitude();

    /**
     * Gets the timezone offset for the most recently determined location.
     *
     * @return     the number of seconds offset from UTC, or 0 if failed
     */
    int getTimezoneOffset();

private:

    bool getCurrentPosition();
    bool getCurrentTimezone();
    bool httpGet(const String hostname, const String url);

    bool hasBeenLocated = false;
    int timezoneOffset;
    float latitude, longitude;
    WiFiClient wifi;
};

extern GeolocationClass Geolocation;
