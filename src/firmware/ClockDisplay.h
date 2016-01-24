#pragma once

#include <Time.h>
#include <Adafruit_NeoPixel.h>

/**
 * Represents the various animations.
 */
enum Animation {

    ANIM_DISCRETE,
    ANIM_CONTINUOUS,
    ANIM_BAR,

    NUM_ANIMATIONS
};

/**
 * Displays data on the clock.
 */
class ClockDisplayClass {

public:

    /**
     * Constructs a new ClockDisplayClass.
     * 
     * @param[in] numPixels The number of NeoPixels connected
     * @param[in] pin The pin the NeoPixels are connected to
     * @param[in] settings The NeoPixel settings
     */
    ClockDisplayClass(int numPixels, int pin, int settings);

    /**
     * Initializes the clock display.
     */
    void begin();

    /**
     * Updates the clock face with the current time.
     * 
     * @note Run frequently inside loop()
     */
    void update();

    /**
     * Displays the time on the clock face.
     * 
     * @param   t   The time to display
     */
    void displayTime(time_t t);

    /**
     * Sets the brightness of the pixels.
     *
     * @param[in]  unscaledBrightness   The new brightness value between 1 and 100
     */
    void setBrightness(int unscaledBrightness);

private:

    void discreteAnimation(time_t t);
    void continuousAnimation(time_t t);
    void barAnimation(time_t t);

    void printTime(time_t t);
    void updateNightMode(time_t t);

    uint32_t addColors(uint32_t a, uint32_t b);
    uint32_t scaleColor(uint32_t color, float scale);
    uint8_t gamma(uint8_t x);
    uint32_t perceived(uint32_t color);

    Adafruit_NeoPixel pixels;
    float brightness;
};

extern ClockDisplayClass ClockDisplay;
