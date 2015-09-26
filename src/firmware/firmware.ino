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

#define NEOPIXELS_PIN       2
#define NEOPIXELS_NUM       60
#define RED                 0xff0000
#define BLUE                0x0000ff

Adafruit_NeoPixel pixels(NEOPIXELS_NUM, NEOPIXELS_PIN, NEO_GRB + NEO_KHZ800);

InternetTime * timeSource;

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

    Serial.println("Locating...");
    Geolocation::locate();

    Serial.println("Setting time sync provider...");
    timeSource = new InternetTime(TIME_SERVER);
    setSyncInterval(SYNC_INTERVAL);
    setSyncProvider(&getInternetTime);

    Serial.println("Starting NeoPixels...");
    pixels.begin();

    Serial.println("Ready");
}

void loop() {

    static int lastSecond = 0;

    // Send the time over serial when the second changes
    time_t t = now();
    if (lastSecond != second(t)) {

        lastSecond = second(t);

        String message = String(hour(t));
        message += ":";
        message += String(minute(t));
        message += ":";
        message += String(second(t));
        Serial.println(message);
    }

    displayTime();
}

/**
 * Shows the current time on the LED strip.
 */
void displayTime() {

    static int lastMinute = 0;
    static int lastHour = 0;

    time_t t = now();
    int currentMinute = minute(t);
    int currentHour = hourFormat12(t);

    // Only update the strip when the time changes
    if (lastMinute != currentMinute && lastHour != currentHour) {

        int hourPixel = currentHour * 5 + currentMinute / 12;
        
        for (int i = 0; i < NEOPIXELS_NUM; i++) {

            uint32_t color = 0x000000;

            if (i < currentMinute) {
                color |= RED;
            }

            if (i == hourPixel) {
                color |= BLUE;
            }

            pixels.setPixelColor(i, color);
        }

        pixels.show();

        lastMinute = currentMinute;
        lastHour = currentHour;
    }
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
