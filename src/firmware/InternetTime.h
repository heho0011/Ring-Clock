#pragma once

#include <TimeLib.h>
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
     * @return     the number of seconds since 1970, in local time
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
        // These bits are swapped due to endianness conversion
        unsigned    mode:    3;
        unsigned    version: 3;
        unsigned    leap:    2;
        uint8_t     stratum;
        int8_t      poll;
        int8_t      precision;
        uint32_t    root_delay;
        uint32_t    root_dispersion;
        uint32_t    ref_id;
        uint64_t    ref_ts;
        uint64_t    origin_ts;
        uint64_t    recv_ts;
        uint64_t    trans_ts;
    } ntp_packet_t;

    void sendRequest(IPAddress ip);
    time_t parseResponse(ntp_packet_t & packet, uint64_t clientTransmitTime, uint64_t clientReceiveTime);
    int8_t calculatePrecision();
    uint64_t getNtpTimestamp();

    const char* hostname;
    WiFiUDP udp;
    uint64_t lastSyncTime;
    int8_t precision;
};

extern InternetTimeClass InternetTime;
