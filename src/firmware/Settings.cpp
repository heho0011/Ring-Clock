#include "Settings.h"
#include "SettingsPage.h"
#include "Geolocation.h"
#include "timezones.h"
#include <EEPROM.h>

#define SETTINGS_DOMAIN     "clock" // Without the .local ending

#define MAGIC_ADDRESS       511
#define MAGIC_NUMBER        0x42

SettingsClass::SettingsClass()
    : server(80) { }

void SettingsClass::begin() {

    // Initialize the store to default values
    memset(&store, 0, sizeof(store));
    initializeEEPROM();

    mdns.begin(SETTINGS_DOMAIN, WiFi.localIP());

    server.on("/", [this](){
        server.send(200, "text/html", generateSettingsPage());
    });

    server.on("/save", [this](){
        
        if (save()) {
            server.sendHeader("Location", "/?status=success");
        } else {
            server.sendHeader("Location", "/?status=error");
        }
        
        server.send(302);
    });

    server.begin();
    Serial.println("Settings web server is running at http://" SETTINGS_DOMAIN ".local");
}

void SettingsClass::initializeEEPROM() {

    EEPROM.begin(512);
    
    if (EEPROM.read(MAGIC_ADDRESS) != MAGIC_NUMBER) {

        // EEPROM has never been written
        // Set the EEPROM to the default store values
        EEPROM.put(0, store);

        // Mark it as written
        EEPROM.write(MAGIC_ADDRESS, MAGIC_NUMBER);

        EEPROM.commit();

    } else {
        // Get the saved values from EEPROM
        EEPROM.get(0, store);
    }
}

void SettingsClass::handleClients() {

    server.handleClient();
}

void SettingsClass::registerObserver(Key key, ObserverFunction observer) {

    observers[key].push_back(observer);
}

void SettingsClass::unregisterObserver(Key key, ObserverFunction observer) {

    for (auto it = observers[key].begin(); it != observers[key].end(); it++) {
        if (*it == observer) {
            observers[key].erase(it);
        }
    }
}

int SettingsClass::get(Key key) {

    return store[key];
}

void SettingsClass::set(Key key, int value) {

    store[key] = value;
    
    EEPROM.put(key * sizeof(int), value);
    EEPROM.commit();

    // Notify the observers
    for (auto& it : observers[key]) {
        (it)(key, value);
    }
}

String SettingsClass::generateSettingsPage() {

    String page = settingsPage;
    if (server.arg("status").equals("success")) {
        page.replace("$alert", "<div class='alert alert-success' role='alert'>Settings saved successfully!</div>");
    } else if (server.arg("status").equals("error")) {
        page.replace("$alert", "<div class='alert alert-danger' role='alert'>An error occurred when saving the settings.</div>");
    } else {
        page.replace("$alert", "");
    }

    int selectedTimezone = get(SELECTED_TIMEZONE);

    // FIXME: crash keeps occurring due to this loop when
    // requesting / from a browser. Might be running out of heap or stack space.
    // Crash actually occurs a few seconds after the server.send returns.
    /* Error message:

        Exception (28):
        epc1=0x4000bf0e epc2=0x00000000 epc3=0x00000000 excvaddr=0x00000000 depc=0x00000000

        ctx: cont 
        sp: 3fff4890 end: 3fff4b70 offset: 01a0

        >>>stack>>>
        3fff4a30:  3ffe0036 3fff4b10 3fff4b10 40211a7f  
        3fff4a40:  3fff24a0 3fff1bdc 3fff4ac0 402120f8  
        3fff4a50:  3fff24a0 3fff1bdc 3fff4b10 4020cd0b  
        3fff4a60:  00000000 0000003f 0000003f 3fffbf48  
        3fff4a70:  0000004f 00000001 3fffb0e8 0000001f  
        3fff4a80:  00000019 00000000 00080000 ffdfffff  
        3fff4a90:  3ffface0 0000000f 00000000 3fffac98  
        3fff4aa0:  0000000f 0000000e 3fffaed8 0000001f  
        3fff4ab0:  00000001 3fff9fb0 0000000f 00000003  
        3fff4ac0:  3fffb9a8 0000005f 0000001c 4020a2d7  
        3fff4ad0:  3fff1c18 4010169f 3fff3b50 00000001  
        3fff4ae0:  00000000 00000019 4020f228 3fff3b50  
        3fff4af0:  402012e6 00000001 00000001 3fff3b40  
        3fff4b00:  3fffdc20 000003b5 3fff1bdc 4020bcb5  
        3fff4b10:  3ffea778 00000000 000003e8 00010480  
        3fff4b20:  3fff1bfc 3fffa758 3fff1abc 3fff3b40  
        3fff4b30:  3fffdc20 00000000 3fff3b38 40207773  
        3fff4b40:  4020f249 00010035 3fff3b38 40206b0e  
        3fff4b50:  00000000 00000000 3fff3b38 4020f271  
        3fff4b60:  00000000 00000000 3fff3b50 40100120  
        <<<stack<<<

         ets Jan  8 2013,rst cause:1, boot mode:(1,6)
    */

    String options = "";
    for (int i = 0; i < 40; i++) {
        options.concat("<option value='");
        options.concat(i);
        options.concat("'");
        if (i == selectedTimezone) {
            options.concat(" selected");
        }
        options.concat(">");
        options.concat(timezones[i]);
        options.concat("</option>");
    }
    page.replace("$timezones", options);

    return page;
}

bool SettingsClass::save() {

    if (server.hasArg("tz")) {

        int tz = server.arg("tz").toInt();
        set(SELECTED_TIMEZONE, tz);
    }

    return true;
}

SettingsClass Settings;
