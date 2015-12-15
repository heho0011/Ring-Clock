/**
 * The Clock Awakens: an LED-ringed clock with WiFi
 * Author: Alex Cordonnier
 */

#include <ESP8266WiFi.h>
#include <Time.h>
#include "config.h"
#include "Geolocation.h"
#include "InternetTime.h"
#include "ClockDisplay.h"

void setup() {

    Serial.begin(115200);
    Serial.print("\n\n");

    printMacAddress();

    Serial.print("Connecting to ");
    Serial.println(WIFI_NETWORK);

    WiFi.begin(WIFI_NETWORK, WIFI_PASSWORD);

    // Try to connect to WiFi until it succeeds
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("Connected");

    printWiFiInfo();

    Serial.println("Initializing geolocation...");
    Geolocation.begin();

    Serial.println("Initializing internet time...");
    InternetTime.begin(TIME_SERVER, SYNC_INTERVAL);

    Serial.println("Initializing NeoPixels...");
    ClockDisplay.begin();

    Serial.println("Ready");
}

void loop() {

    ClockDisplay.update();
}

/**
 * Prints the ESP8266's MAC address for reference.
 */
void printMacAddress() {

    Serial.print("MAC address: ");

    unsigned char mac[WL_MAC_ADDR_LENGTH];
    WiFi.macAddress(mac);

    for (int i = WL_MAC_ADDR_LENGTH - 1; i > 0; i--) {
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
