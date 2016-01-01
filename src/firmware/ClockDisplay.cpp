#include "ClockDisplay.h"
#include "DataStore.h"

#define NEOPIXELS_PIN       2
#define NEOPIXELS_NUM       60

#define DEBUG               1

#define EXTRACT_RED(c)      (((c) & 0xff0000) >> 16)
#define EXTRACT_GREEN(c)    (((c) & 0x00ff00) >> 8)
#define EXTRACT_BLUE(c)     ((c) & 0x0000ff)

#define GAMMA               2.8

void onBrightnessUpdate(DSKey key, int value) {

    Serial.println("Brightness updated!");
    ClockDisplay.setBrightness(value);
}

bool brightnessValidator(DSKey key, int value) {

    // Make sure that the value is between 0 and 100
    return (value >= 0 && value <= 100);
}

bool colorValidator(DSKey key, int value) {

    // Make sure the value is 24 bits
    return (value <= 0xffffff && value >= 0x000000);
}

ClockDisplayClass::ClockDisplayClass(int numPixels, int pin, int settings)
    : pixels(numPixels, pin, settings) { }

void ClockDisplayClass::begin() {

    pixels.begin();

    setBrightness(DataStore.get(DS_BRIGHTNESS));
    DataStore.registerObserver(DS_BRIGHTNESS, &onBrightnessUpdate);

    DataStore.registerValidator(DS_BRIGHTNESS, &brightnessValidator);
    DataStore.registerValidator(DS_HOUR_COLOR, &colorValidator);
    DataStore.registerValidator(DS_MINUTE_COLOR, &colorValidator);
    DataStore.registerValidator(DS_SECOND_COLOR, &colorValidator);
}

void ClockDisplayClass::update() {

    displayTime(now());
}

void ClockDisplayClass::displayTime(time_t t) {

    static int lastSecond = 0;

    int currentSecond = second(t);
    int currentMinute = minute(t);
    int currentHour = hour(t);

    // Only update the strip when the time changes
    if (lastSecond != currentSecond) {

#ifdef DEBUG
        String message = String(currentHour);
        message += ":";
        message += String(currentMinute);
        message += ":";
        message += String(currentSecond);
        Serial.println(message);
#endif

        int hourPixel = (currentHour % 12) * 5 + currentMinute / 12;
        
        for (int i = 0; i < NEOPIXELS_NUM; i++) {

            uint32_t color = 0x000000;

            if (i == currentSecond) {
                color |= DataStore.get(DS_SECOND_COLOR);
            }

            if (i == currentMinute) {
                color |= DataStore.get(DS_MINUTE_COLOR);
            }

            if (i == hourPixel) {
                color |= DataStore.get(DS_HOUR_COLOR);
            }

            pixels.setPixelColor(i, perceived(color));
        }

        pixels.show();

        lastSecond = currentSecond;
    }
}

void ClockDisplayClass::setBrightness(int brightness) {

    // Make sure input is valid
    if (brightness < 0 || brightness > 100) {
        return;
    }

    // Scale the brightness 
    uint8_t scaledBrightness = brightness * 255 / 100;
    pixels.setBrightness(gamma(scaledBrightness));
}

uint8_t ClockDisplayClass::gamma(uint8_t x) {

    // See https://learn.adafruit.com/led-tricks-gamma-correction/the-issue
    return (uint8_t)(0.5 + 255.0 * pow(x / 255.0, GAMMA));
}

uint32_t ClockDisplayClass::perceived(uint32_t color) {

    uint8_t red = gamma(EXTRACT_RED(color));
    uint8_t green = gamma(EXTRACT_GREEN(color));
    uint8_t blue = gamma(EXTRACT_BLUE(color));

    return pixels.Color(red, green, blue);
}

ClockDisplayClass ClockDisplay(NEOPIXELS_NUM, NEOPIXELS_PIN, NEO_GRB + NEO_KHZ800);
