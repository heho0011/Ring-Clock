/**
 * The Clock Awakens: an LED-ringed clock with WiFi
 * Author: Alex Cordonnier
 */

#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>
#include <Time.h>
#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>

#include "config.h"
#include "DataStore.h"
#include "WebServer.h"
#include "Geolocation.h"
#include "InternetTime.h"
#include "ClockDisplay.h"

void setup() {

    Serial.begin(115200);
    Serial.print("\n\n");

    printMacAddress();

    WiFiManager wifiManager;
    wifiManager.autoConnect("Clock Setup");

    Serial.println("");
    Serial.println("Connected");

    printWiFiInfo();

    DataStore.begin();
    WebServer.begin();
    Geolocation.begin();
    InternetTime.begin(TIME_SERVER, SYNC_INTERVAL);
    ClockDisplay.begin();

    Serial.println("Ready");
}

void loop() {

    ClockDisplay.update();
    
    WebServer.handleClients();
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
