#pragma once

#include <Time.h>
#include "Adafruit_NeoPixel.h"

/**
 * A class to retrieve the current time from the internet.
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

private:

    Adafruit_NeoPixel pixels;
};

extern ClockDisplayClass ClockDisplay;
