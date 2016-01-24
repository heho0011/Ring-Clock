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
     * Gets the time of sunrise for the most recently determined location.
     *
     * @return     the timestamp, or 0 if failed
     */
    time_t getSunriseTime();

    /**
     * Gets the time of sunset for the most recently determined location.
     *
     * @return     the timestamp, or 0 if failed
     */
    time_t getSunsetTime();

    /**
     * Gets the detected timezone code.
     *
     * @return     the timezone
     */
    String getDetectedTimezone();

    /**
     * Gets the timezone offset for the most recently determined location.
     * 
     * param[in] force Whether to force an update. Defaults to false.
     *
     * @return     the number of seconds offset from UTC, or 0 if failed
     */
    int getTimezoneOffset(bool force = false);

private:

    time_t extractTime(String utc);
    bool updatePosition();
    bool updateSunriseSunset();
    bool updateTimezone();
    bool httpGet(const String hostname, const String url);

    WiFiClient wifi;

    int timezoneOffset;
    float latitude, longitude;
    String detectedTimezone;

    time_t sunriseTime, sunsetTime;
};

extern GeolocationClass Geolocation;
