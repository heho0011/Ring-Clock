/**
 * The Clock Awakens: an LED-ringed clock with WiFi
 * Author: Alex Cordonnier
 */

#include <ESP8266WiFi.h>
#include <Time.h>
#include <Adafruit_NeoPixel.h>
#include "Geolocation.h"
#include "InternetTime.h"
#include "config.h"

#define NEOPIXELS_PIN        6
#define NEOPIXELS_NUM       60

Adafruit_NeoPixel pixels(NEOPIXELS_NUM, NEOPIXELS_PIN, NEO_GRB | NEO_KHZ800);

InternetTime * timeSource;

void setup() {

    Serial.begin(115200);

    // Serial.println("Starting NeoPixels...");
    // pixels.begin();


    printMacAddress();

    Serial.print("Connecting to ");
    Serial.println(WIFI_NETWORK);

    WiFi.begin(WIFI_NETWORK, WIFI_PASSWORD);

    // Try to connect to WiFi until it succeeds
    while (WiFi.status() != WL_CONNECTED) {
        // Spin here until it connects
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
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
