#include "ClockDisplay.h"
#include "DataStore.h"

#define NEOPIXELS_PIN       2
#define NEOPIXELS_NUM       60

#define EXTRACT_RED(c)      (((c) & 0xff0000) >> 16)
#define EXTRACT_GREEN(c)    (((c) & 0x00ff00) >> 8)
#define EXTRACT_BLUE(c)     ((c) & 0x0000ff)

#define GAMMA               2.2
#define FPS                 20

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

bool animationValidator(DSKey key, int value) {

    // Make sure the value is within the enum bounds
    return (value < NUM_ANIMATIONS && value >= 0);
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
    DataStore.registerValidator(DS_CLOCK_ANIMATION, &animationValidator);
}

void ClockDisplayClass::update() {

    displayTime(now());
}

void ClockDisplayClass::displayTime(time_t t) {

    printTime(t);

    int selectedAnimation = DataStore.get(DS_CLOCK_ANIMATION);

    switch(selectedAnimation) {

        case ANIM_DISCRETE:
            discreteAnimation(t);
            break;

        case ANIM_CONTINUOUS:
            continuousAnimation(t);
            break;

        case ANIM_BAR:
            barAnimation(t);
            break;
    }
}

void ClockDisplayClass::discreteAnimation(time_t t) {

    static int lastSecond = -1;

    int currentSecond = second(t);
    int currentMinute = minute(t);
    int currentHour = hour(t);

    // Only update the strip when the time changes
    if (lastSecond != currentSecond) {

        int hourPixel = (currentHour % 12) * 5 + currentMinute / 12;
        
        for (int i = 0; i < NEOPIXELS_NUM; i++) {

            uint32_t color = 0x000000;

            if (i == currentSecond) {
                color = addColors(color, DataStore.get(DS_SECOND_COLOR));
            }

            if (i == currentMinute) {
                color = addColors(color, DataStore.get(DS_MINUTE_COLOR));
            }

            if (i == hourPixel) {
                color = addColors(color, DataStore.get(DS_HOUR_COLOR));
            }

            pixels.setPixelColor(i, perceived(color));
        }

        pixels.show();

        lastSecond = currentSecond;
    }
}

void ClockDisplayClass::continuousAnimation(time_t t) {

    static int lastSecond = -1;
    static unsigned long lastAbsoluteMillis = 0;
    static int millisAtSecondStart = 0;

    unsigned long currentAbsoluteMillis = millis();
    int currentMillis = (currentAbsoluteMillis - millisAtSecondStart) % 1000;
    int currentSecond = second(t);
    int currentMinute = minute(t);
    int currentHour = hour(t);

    if (lastSecond != currentSecond) {

        lastSecond = currentSecond;
        millisAtSecondStart = currentAbsoluteMillis % 1000;
    }

    // Only update the strip when the frame changes
    if (currentAbsoluteMillis - lastAbsoluteMillis >= 1000/FPS) {

        int currentSecondPixel = currentSecond;
        int nextSecondPixel = (currentSecondPixel + 1) % 60;
        int currentMinutePixel = currentMinute;
        int nextMinutePixel = (currentMinutePixel + 1) % 60;
        int currentHourPixel = (currentHour % 12) * 5 + currentMinute / 12;
        int nextHourPixel = (currentHourPixel + 1) % 60;

        float percentSinceCurrentSecondPixel = currentMillis / 1000.0;
        float percentUntilNextSecondPixel = 1 - percentSinceCurrentSecondPixel;
        float percentSinceCurrentMinutePixel = currentSecond / 60.0;
        float percentUntilNextMinutePixel = 1 - percentSinceCurrentMinutePixel;
        float percentSinceCurrentHourPixel = (currentMinute % 12) / 12.0;
        float percentUntilNextHourPixel = 1 - percentSinceCurrentHourPixel;
        
        for (int i = 0; i < NEOPIXELS_NUM; i++) {

            uint32_t color = 0x000000;

            if (i == currentSecondPixel) {
                color = addColors(color, scaleColor(perceived(DataStore.get(DS_SECOND_COLOR)), percentUntilNextSecondPixel));
            }

            if (i == nextSecondPixel) {
                color = addColors(color, scaleColor(perceived(DataStore.get(DS_SECOND_COLOR)), percentSinceCurrentSecondPixel));
            }

            if (i == currentMinutePixel) {
                color = addColors(color, scaleColor(perceived(DataStore.get(DS_MINUTE_COLOR)), percentUntilNextMinutePixel));
            }

            if (i == nextMinutePixel) {
                color = addColors(color, scaleColor(perceived(DataStore.get(DS_MINUTE_COLOR)), percentSinceCurrentMinutePixel));
            }

            if (i == currentHourPixel) {
                color = addColors(color, scaleColor(perceived(DataStore.get(DS_HOUR_COLOR)), percentUntilNextHourPixel));
            }

            if (i == nextHourPixel) {
                color = addColors(color, scaleColor(perceived(DataStore.get(DS_HOUR_COLOR)), percentSinceCurrentHourPixel));
            }

            pixels.setPixelColor(i, color);
        }

        pixels.show();

        lastAbsoluteMillis = currentAbsoluteMillis;
    }
}

void ClockDisplayClass::barAnimation(time_t t) {

    static int lastMinute = -1;

    int currentSecond = second(t);
    int currentMinute = minute(t);
    int currentHour = hour(t);

    // Only update the strip when the time changes
    if (lastMinute != currentMinute) {

        int hourPixel = (currentHour % 12) * 5 + currentMinute / 12;
        
        for (int i = 0; i < NEOPIXELS_NUM; i++) {

            uint32_t color = 0x000000;

            if (i <= currentMinute) {
                color = addColors(color, DataStore.get(DS_MINUTE_COLOR));
            }

            if (i == hourPixel) {
                color = addColors(color, DataStore.get(DS_HOUR_COLOR));
            }

            pixels.setPixelColor(i, perceived(color));
        }

        pixels.show();

        lastMinute = currentMinute;
    }
}

void ClockDisplayClass::printTime(time_t t) {

    static int lastSecond = -1;

    int currentSecond = second(t);
    int currentMinute = minute(t);
    int currentHour = hour(t);

    if (lastSecond != currentSecond) {

        Serial.println(String(currentHour) + ":" +
                       String(currentMinute) + ":" +
                       String(currentSecond));
        
        lastSecond = currentSecond;
    }
}

uint32_t ClockDisplayClass::addColors(uint32_t a, uint32_t b) {

    uint8_t red = std::min(EXTRACT_RED(a) + EXTRACT_RED(b), (uint32_t)0xff);
    uint8_t green = std::min(EXTRACT_GREEN(a) + EXTRACT_GREEN(b), (uint32_t)0xff);
    uint8_t blue = std::min(EXTRACT_BLUE(a) + EXTRACT_BLUE(b), (uint32_t)0xff);

    return pixels.Color(red, green, blue);
}

uint32_t ClockDisplayClass::scaleColor(uint32_t color, float scale) {

    uint8_t red = constrain(scale * EXTRACT_RED(color), 0, 255);
    uint8_t green = constrain(scale * EXTRACT_GREEN(color), 0, 255);
    uint8_t blue = constrain(scale * EXTRACT_BLUE(color), 0, 255);

    return pixels.Color(red, green, blue);
}

void ClockDisplayClass::setBrightness(int unscaledBrightness) {

    // Make sure input is valid
    if (!brightnessValidator(DS_BRIGHTNESS, unscaledBrightness)) {
        return;
    }

    // Scale the brightness 
    brightness = unscaledBrightness / 100.0;
}

uint8_t ClockDisplayClass::gamma(uint8_t x) {

    // See https://learn.adafruit.com/led-tricks-gamma-correction/the-issue
    return (uint8_t)(0.5 + 255.0 * pow(x / 255.0, GAMMA));
}

uint32_t ClockDisplayClass::perceived(uint32_t color) {

    color = scaleColor(color, brightness);

    uint8_t red = gamma(EXTRACT_RED(color));
    uint8_t green = gamma(EXTRACT_GREEN(color));
    uint8_t blue = gamma(EXTRACT_BLUE(color));

    return pixels.Color(red, green, blue);
}

ClockDisplayClass ClockDisplay(NEOPIXELS_NUM, NEOPIXELS_PIN, NEO_GRB + NEO_KHZ800);
