#include "ClockDisplay.h"

#define NEOPIXELS_PIN       2
#define NEOPIXELS_NUM       60

#define RED                 0xff0000
#define GREEN               0x00ff00
#define BLUE                0x0000ff

Adafruit_NeoPixel ClockDisplay::pixels(NEOPIXELS_NUM,
                                       NEOPIXELS_PIN,
                                       NEO_GRB + NEO_KHZ800);

/**
 * Sets up the NeoPixel strip.
 */
void ClockDisplay::init() {
    pixels.begin();
}

/**
 * Shows the given time on the NeoPixel strip.
 */
void ClockDisplay::displayTime(time_t t) {

    static int lastMinute = 0;
    static int lastHour = 0;

    int currentMinute = minute(t);
    int currentHour = hourFormat12(t);

    // Only update the strip when the time changes
    if (lastMinute != currentMinute || lastHour != currentHour) {

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