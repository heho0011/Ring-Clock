Ring Clock forked from ajcord/The-Clock-Awakens
===============================================

Using LEDs and an ESP8266 to make a smart digital analog clock.

Uses the [Sunrise-Sunset API](http://sunrise-sunset.org/api) among others.

Only modified the code so that it works with the latest Arduino IDE 1.6.11 and up-to-date libraries.

Setting up the environment
--------------------------

1. Set up the ESP8266 in the Arduino IDE ([instructions](https://github.com/esp8266/Arduino#installing-with-boards-manager))
2. Install the [TimeLib](https://github.com/PaulStoffregen/Time), [Adafruit NeoPixel](https://github.com/adafruit/Adafruit_NeoPixel) and [WiFiManager](https://github.com/tzapu/WiFiManager) libraries through the Library Manager ([instructions](https://www.arduino.cc/en/Guide/Libraries#toc3))
3. Install the [ESP8266FS tool](https://github.com/esp8266/arduino-esp8266fs-plugin/releases/download/0.2.0/ESP8266FS-0.2.0.zip) ([instructions](https://github.com/esp8266/arduino-esp8266fs-plugin))
4. Make the necessary changes to `src/firmware/config.h.template` and save them in `src/firmware/config.h`
5. Open `src/firmware/firmware.ino` in the Arduino IDE
6. Upload the sketch data by going to Tools > ESP8266 Sketch Data Upload
7. Upload the sketch to the device
