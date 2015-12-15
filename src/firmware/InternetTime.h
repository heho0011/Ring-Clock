#pragma once

#include <Time.h>
#include <WiFiUdp.h>

/**
 * A class to retrieve the current time from the internet.
 */
class InternetTimeClass {

public:

    /**
     * Constructs a new internet time source (NTP).
     */
    InternetTimeClass();

    /**
     * Begins checking the time.
     * 
     * @param[in]  timeServer  The hostname of the time server to use
     * @param[in]  interval  The number of seconds between sync intervals
     */
    void begin(const char* timeServer, time_t interval);

    /**
     * Gets the time from the server.
     *
     * @return     the number of seconds since 1970, in UTC
     */
    time_t getTime();

private:

    // The format of an NTP timestamp: whole seconds followed by fractional
    typedef struct timestamp {
        uint32_t ts_seconds;
        uint32_t ts_fraction;
    } timestamp_t;

    // See RFC 1305 for details on the transmit packet structure.
    typedef struct ntp_packet {
        unsigned int leap_indicator: 2;
        unsigned int version_number: 3;
        unsigned int mode: 3;
        uint8_t  stratum;
        uint8_t  poll;
        uint8_t  precision;
        uint32_t root_delay;
        uint32_t root_dispersion;
        uint32_t  ref_id;
        timestamp_t ref_ts;
        timestamp_t origin_ts;
        timestamp_t recv_ts;
        timestamp_t trans_ts;
    } ntp_packet_t;

    void sendRequest(ntp_packet_t & buffer, IPAddress ip);
    time_t parseResponse(ntp_packet_t & buffer);

    const char* hostname;
    WiFiUDP udp;
    timestamp_t lastSyncTime;
};

extern InternetTimeClass InternetTime;
