#pragma once

#include <ESP8266WiFi.h>

/**
 * A class to get the current position and time zone.
 */
class Geolocation {

public:

    /**
     * Determines the approximate current position.
     *
     * @return     whether the user was located successfully
     */
    static bool locate();

    /**
     * Gets the most recently determined latitude.
     *
     * @return     the latitude
     */
    static float getLatitude();

    /**
     * Gets the most recently determined longitude.
     *
     * @return     the latitude
     */
    static float getLongitude();

    /**
     * Gets the timezone offset for the most recently determined location.
     *
     * @return     the number of seconds offset from UTC, or 0 if failed
     */
    static int getTimezoneOffset();

private:

    static bool getCurrentPosition();
    static bool getCurrentTimezone();
    static bool httpGet(const String hostname, const String url);

    static int timezoneOffset;
    static float latitude, longitude;
    static WiFiClient wifi;
};