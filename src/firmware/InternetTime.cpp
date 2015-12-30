#include "InternetTime.h"
#include "Geolocation.h"
#include <ESP8266WiFi.h>

#define HTTP_PORT           80
#define UDP_IN_PORT         2390
#define NTP_OUT_PORT        123
#define TIMEOUT             2000 // ms

#define UNIX_TIME_OFFSET    2208988800UL // Number of seconds from 1900 to 1970
#define NTP_TO_MICROS_RATIO 4295 // Roughly (2^32 / 1000000)
#define PRECISION_TEST_NUM_ITERATIONS   100

// Leap indicator warning bits
enum {
    NTP_LI_NONE,    // No leap second
    NTP_LI_INSERT,  // Last minute has 61 seconds
    NTP_LI_DELETE,  // Last minute has 59 seconds
    NTP_LI_UNKNOWN,   // Clock not synchronized
} NTP_LI;

// NTP mode bits
enum {
    NTP_MODE_RESERVED,
    NTP_MODE_SYMMETRIC_ACTIVE,
    NTP_MODE_SYMMETRIC_PASSIVE,
    NTP_MODE_CLIENT,
    NTP_MODE_SERVER,
    NTP_MODE_BROADCAST,
    NTP_MODE_CONTROL_MESSAGE,
    NTP_MODE_RESERVED_PRIVATE,
} NTP_MODE;

uint32_t extractSeconds(uint64_t ntp) {

    return (uint32_t)((ntp >> 32) & 0xFFFFFFFF);
}

uint32_t extractFraction(uint64_t ntp) {

    return (uint32_t)(ntp & 0xFFFFFFFF);
}

double fixedToFloat(uint64_t ntp) {

    uint32_t seconds = extractSeconds(ntp);
    uint32_t fraction = extractFraction(ntp);

    return (double)seconds + (double)fraction/(1ULL << 32);
}

InternetTimeClass::InternetTimeClass() {

    lastSyncTime = 0;
}

time_t updateTime() {
    return InternetTime.getTime();
}

void InternetTimeClass::begin(const char* timeServer, time_t interval) {

    hostname = timeServer;

    precision = calculatePrecision();

    // Start listening on the incoming UDP port
    udp.begin(UDP_IN_PORT);

    // Set up time library
    setSyncInterval(interval);
    setSyncProvider(&updateTime);
}

int8_t InternetTimeClass::calculatePrecision() {
    
    unsigned long start = micros();

    for (int i = 0; i < PRECISION_TEST_NUM_ITERATIONS; i++) {
        micros();
    }

    unsigned long end = micros();
    unsigned long totalTime = end - start;
    unsigned long averageTime = totalTime/PRECISION_TEST_NUM_ITERATIONS;

    if (averageTime == 0) {
        averageTime = 1;
    }

    int8_t precision = log2(averageTime/1000000.0);

    return precision;
}

uint64_t InternetTimeClass::getNtpTimestamp() {

    // Comes up with a reasonable NTP-formatted timestamp based on the number
    // of microseconds since startup.
    // Should only be used to compute time intervals, not absolute times.
    // Can't use now() here because it will infinitely recurse.
    return (uint64_t)((double)micros()/1000000.0 * (1ULL << 32));
}

time_t InternetTimeClass::getTime() {

    // Get the timezone first to avoid interfering with NTP calculations
    time_t timezoneOffset = Geolocation.getTimezoneOffset();

    // Perform a DNS lookup on the hostname
    IPAddress ip;
    int dnsError = WiFi.hostByName(hostname, ip);
    if (dnsError != 1) {
        Serial.print("Could not resolve hostname '");
        Serial.print(hostname);
        Serial.print("'. Error: ");
        Serial.println(dnsError);
        return 0;
    }

    sendRequest(ip);
    uint64_t clientTransmitTime = getNtpTimestamp();

    long lastRequestTime = millis();
    int numberOfTimeouts = 0;

    // Wait for a response
    while (!udp.parsePacket()) {

        if (millis() - lastRequestTime > (TIMEOUT << numberOfTimeouts)) {

            Serial.println("NTP request timed out.");
            sendRequest(ip);
            clientTransmitTime = getNtpTimestamp();
            lastRequestTime = millis();
            numberOfTimeouts++;
        }

        yield(); // Reset the watchdog
    }
    
    ntp_packet_t response;
    uint64_t clientReceiveTime = getNtpTimestamp();
    udp.read((char*)&response, sizeof(ntp_packet_t));
    time_t currentTime = parseResponse(response, clientTransmitTime, clientReceiveTime);

    // Convert from UTC to localized time
    time_t localizedTime = currentTime + timezoneOffset;

    Serial.print("The current local time is: ");
    Serial.println(localizedTime);

    return localizedTime;
}

void InternetTimeClass::sendRequest(IPAddress ip) {

    ntp_packet_t packet;
    memset(&packet, 0, sizeof(ntp_packet_t));

    // Build the NTP packet
    packet.leap = NTP_LI_UNKNOWN;
    packet.version = 4;
    packet.mode = NTP_MODE_CLIENT;
    packet.stratum = 16;
    packet.poll = 6;
    packet.precision = precision;
    packet.ref_id = ip;
    packet.ref_ts = lastSyncTime;
    packet.origin_ts = __builtin_bswap64(getNtpTimestamp());

    udp.beginPacket(ip, NTP_OUT_PORT);
    udp.write((char*)&packet, sizeof(ntp_packet_t));
    udp.endPacket();
}

time_t InternetTimeClass::parseResponse(ntp_packet_t & packet,
    uint64_t clientTransmitTime, uint64_t clientReceiveTime) {

    // Remember to convert big-endian network values to
    // little-endian host values
    uint64_t serverReceiveTime = __builtin_bswap64(packet.recv_ts);
    uint64_t serverTransmitTime = __builtin_bswap64(packet.trans_ts);

    // The number of seconds we are ahead of the server
    uint64_t offset = ((serverReceiveTime - clientTransmitTime) +
                    (serverTransmitTime - clientReceiveTime))/2;

    // The round-trip delay on the network, assuming symmetry
    uint64_t delay = ((clientReceiveTime - clientTransmitTime) -
                    (serverTransmitTime - serverReceiveTime))/2;

    uint64_t currentTime = serverTransmitTime + delay;

    lastSyncTime = currentTime;

    // For the highest accuracy, wait until the second is just changing
    // before returning the time since it gets rounded to seconds by Time.h
    uint64_t timeElapsed = getNtpTimestamp() - clientReceiveTime;
    uint64_t timeSinceSecondStarted = (currentTime + timeElapsed) & 0xFFFFFFFF;
    uint64_t timeLeftInSecond = 0x100000000 - timeSinceSecondStarted;
    time_t microsecondsToWait = (time_t)(timeLeftInSecond / NTP_TO_MICROS_RATIO);
    delayMicroseconds(microsecondsToWait);

    // Convert the wire time, which uses an epoch of 1900,
    // to Unix time, which uses an epoch of 1970.
    currentTime = currentTime + timeElapsed + timeLeftInSecond;
    time_t unixTime = extractSeconds(currentTime) - UNIX_TIME_OFFSET;

    return unixTime;
}

InternetTimeClass InternetTime;
