/**
 * The Clock Awakens: an LED-ringed clock with WiFi
 * Author: Alex Cordonnier
 */

#include <ESP8266WiFi.h>
#include <Time.h>
#include "Geolocation.h"
#include "InternetTime.h"
#include "config.h"

InternetTime * timeSource;

void setup() {

    Serial.begin(9600);

    printMacAddress();

    // Try to connect to WiFi until it succeeds
    while (WiFi.begin(WIFI_NETWORK, WIFI_PASSWORD) != WL_CONNECTED) {

        Serial.print("Could not connect to \"");
        Serial.print(WIFI_NETWORK);
        Serial.println("\". Retrying in 10 seconds...");
        delay(10000);
    }

    Serial.println("Connected");

    printWiFiInfo();

    Serial.println("Locating...");
    Geolocation::locate();

    Serial.println("Setting time sync provider...");
    timeSource = new InternetTime(TIME_SERVER);
    setSyncInterval(SYNC_INTERVAL);
    setSyncProvider(&getInternetTime);

    Serial.println("Ready");
}

void loop() {

    delay(1000);
    Serial.print(".");
}

/**
 * Wraps the InternetTime read function.
 *
 * @return     the time from the internet time source
 */
time_t getInternetTime() {
    return timeSource->getTime();
}

/**
 * Prints the ESP8266's MAC address for reference.
 */
void printMacAddress() {

    Serial.print("MAC address: ");

    unsigned char mac[6];
    WiFi.macAddress(mac);

    for (int i = 5; i > 0; i--) {
        Serial.print(mac[i], HEX);
        Serial.print(":");
    }

    Serial.println(mac[0], HEX);
}

/**
 * Prints info about the WiFi connection.
 */
void printWiFiInfo() {

    Serial.print("Local IP address: ");
    Serial.println(WiFi.localIP());

    Serial.print("Signal strength (RSSI): ");
    Serial.println(WiFi.RSSI());
}
