#pragma once

#include <Arduino.h>
#include <vector>

/**
 * Represents the various possible keys.
 */
enum DSKey {

    DS_TIMEZONE,
    DS_BRIGHTNESS,
    DS_HOUR_COLOR,
    DS_MINUTE_COLOR,
    DS_SECOND_COLOR,
    DS_CLOCK_ANIMATION,
    DS_NIGHT_MODE_ENABLE,
    DS_NIGHT_MODE_BRIGHTNESS,
    DS_GAMMA,
    DS_RESERVED1,
    DS_RESERVED2,
    DS_LAST_TIMEZONE_OFFSET,

    DS_NUM_KEYS
};

/**
 * Manages key-value pairs and provides an interface for changing them.
 */
class DataStoreClass {
    
public:

    /**
     * Initializes the store.
     */
    void begin();

    /**
     * Formats the EEPROM, restoring all initial values.
     */
    void resetSettings();

    /**
     * Function signature for an observer function.
     * Receives the key and new value as parameters and returns nothing.
     */
    typedef void (*ObserverFunction)(DSKey, int);

    /**
     * Function signature for a validator function.
     * Receives the key and proposed value as parameters and returns true
     * if the value should be accepted or false otherwise.
     */
    typedef bool (*ValidatorFunction)(DSKey, int);

    /**
     * Registers a function to be called when the given key's value is updated.
     *
     * @param      key       The key to observe
     * @param[in]  observer  The function to be called. See above.
     */
    void registerObserver(DSKey key, ObserverFunction observer);

    /**
     * Unregisters all instances of an observer from a key.
     *
     * @param      key       The key being observed
     * @param[in]  observer  The function to unregister
     */
    void unregisterObserver(DSKey key, ObserverFunction observer);

    /**
     * Registers a function to be called to verify the given key's new value.
     *
     * @param      key       The key to validate
     * @param[in]  validator The function to be called. See above.
     */
    void registerValidator(DSKey key, ValidatorFunction observer);

    /**
     * Unregisters all instances of an observer from a key.
     *
     * @param      key       The key being validated
     * @param[in]  validator The function to unregister
     */
    void unregisterValidator(DSKey key, ValidatorFunction observer);

    /**
     * Gets a value.
     *
     * @param[in]  key    The key to get
     *
     * @return     The key's value
     */
    int get(DSKey key);

    /**
     * Sets a value.
     *
     * @param[in]  key   The key to set
     * @param[in]  key   The value to change it to
     * 
     * @return      Whether the value was acceptable
     */
    bool set(DSKey key, int value);

private:

    int store[DS_NUM_KEYS];
    std::vector<ObserverFunction> observers[DS_NUM_KEYS];
    std::vector<ValidatorFunction> validators[DS_NUM_KEYS];
};

extern DataStoreClass DataStore;
