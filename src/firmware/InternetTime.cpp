#include "InternetTime.h"
#include "Geolocation.h"
#include <ESP8266WiFi.h>

#define HTTP_PORT           80
#define UDP_IN_PORT         2390
#define NTP_OUT_PORT        123
#define UNIX_TIME_OFFSET    2208988800L

// Leap indicator warning bits
enum {
    NTP_LI_NONE,    // No leap second
    NTP_LI_INSERT,  // Last minute has 61 seconds
    NTP_LI_DELETE,  // Last minute has 59 seconds
    NTP_LI_ALARM,   // Clock not synchronized
} NTP_LI;

// NTP mode bits
enum {
    NTP_MODE_RESERVED,
    NTP_MODE_SYMMETRIC_ACTIVE,
    NTP_MODE_SYMMETRIC_PASSIVE,
    NTP_MODE_CLIENT,
    NTP_MODE_SERVER,
    NTP_MODE_BROADCAST,
    NTP_MODE_RESERVED_CONTROL,
} NTP_MODE;

InternetTime::InternetTime(const char* timeServer) {

    hostname = timeServer;
    memset(&lastSyncTime, 0, sizeof(timestamp_t));

    // Start listening on the incoming UDP port
    udp.begin(UDP_IN_PORT);
}

time_t InternetTime::getTime() {

    static ntp_packet_t packet;

    Serial.print("Resolving hostname \'");
    Serial.print(hostname);
    Serial.println("\'...");

    // Perform a DNS lookup on the hostname
    IPAddress ip;
    int dnsError = WiFi.hostByName(hostname, ip);
    if (dnsError) {
        Serial.print("Could not resolve hostname. Error: ");
        Serial.println(dnsError);
        return 0;
    }

    Serial.print("Resolved to ");
    Serial.println(ip);


    Serial.println("Sending NTP request...");
    sendRequest(packet, ip);


    // Wait for a response
    Serial.println("Waiting for response...");
    while (!udp.parsePacket()) { }

    udp.read((char*)&packet, sizeof(ntp_packet_t));
    
    time_t currentTime = parseResponse(packet);

    Serial.print("The current local time is: ");
    Serial.println(currentTime);

    return currentTime;
}

void InternetTime::sendRequest(ntp_packet_t & packet, IPAddress ip) {

    memset(&packet, 0, sizeof(ntp_packet_t));

    packet.leap_indicator = NTP_LI_ALARM;
    packet.version_number = 4;
    packet.mode = NTP_MODE_CLIENT;
    packet.stratum = 2;
    packet.poll = 6;
    packet.precision = -14;
    packet.ref_id = ip;
    packet.ref_ts = lastSyncTime;
    packet.origin_ts.ts_seconds = now() + UNIX_TIME_OFFSET;

    udp.beginPacket(ip, NTP_OUT_PORT);
    udp.write((char*)&packet, sizeof(ntp_packet_t));
    udp.endPacket();
}

time_t InternetTime::parseResponse(ntp_packet_t & packet) {

    // Convert the wire time, which uses an epoch of 1900,
    // to Unix time, which uses an epoch of 1970.
    time_t unixTime = packet.recv_ts.ts_seconds - UNIX_TIME_OFFSET;

    // Convert from UTC to localized time
    time_t timezoneOffset = Geolocation::getTimezoneOffset();
    time_t localizedTime = unixTime + timezoneOffset;

    return localizedTime;
}
