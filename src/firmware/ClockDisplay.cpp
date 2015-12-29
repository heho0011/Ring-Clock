#include "ClockDisplay.h"
#include "Settings.h"

#define NEOPIXELS_PIN       2
#define NEOPIXELS_NUM       60

#define DEBUG               1

#define EXTRACT_RED(c)      ((c & 0xff0000) >> 16)
#define EXTRACT_GREEN(c)    ((c & 0x00ff00) >> 8)
#define EXTRACT_BLUE(c)     (c & 0x0000ff)

#define GAMMA               2.2

void onBrightnessUpdate(Key key, int value) {

    Serial.println("Brightness updated!");
    ClockDisplay.setBrightness(value);
}

ClockDisplayClass::ClockDisplayClass(int numPixels, int pin, int settings)
    : pixels(numPixels, pin, settings) { }

void ClockDisplayClass::begin() {

    pixels.begin();

    setBrightness(Settings.get(SET_BRIGHTNESS));
    Settings.registerObserver(SET_BRIGHTNESS, &onBrightnessUpdate);
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
                color |= Settings.get(SET_SECOND_COLOR);
            }

            if (i == currentMinute) {
                color |= Settings.get(SET_MINUTE_COLOR);
            }

            if (i == hourPixel) {
                color |= Settings.get(SET_HOUR_COLOR);
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
    pixels.setBrightness(scaledBrightness);
}

uint8_t ClockDisplayClass::gamma(uint8_t x) {

    return (uint8_t)(255 * pow(x / 255.0, 1 / GAMMA));
}

uint32_t ClockDisplayClass::perceived(uint32_t color) {

    uint8_t red = gamma(EXTRACT_RED(color));
    uint8_t green = gamma(EXTRACT_GREEN(color));
    uint8_t blue = gamma(EXTRACT_BLUE(color));

    return pixels.Color(red, green, blue);
}

ClockDisplayClass ClockDisplay(NEOPIXELS_NUM, NEOPIXELS_PIN, NEO_GRB + NEO_KHZ800);
