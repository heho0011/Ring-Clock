#pragma once

#include <Time.h>
#include "Adafruit_NeoPixel.h"

/**
 * A class to retrieve the current time from the internet.
 */
class ClockDisplay {

public:

    /**
     * Initializes the clock display.
     */
    static void init();

    /**
     * Displays the time on the clock face.
     * 
     * @param   t   The time to display
     */
    static void displayTime(time_t t);

private:

    static Adafruit_NeoPixel pixels;
};
