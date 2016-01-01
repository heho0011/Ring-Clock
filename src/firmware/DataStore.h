#pragma once

#include <Arduino.h>
#include <vector>

/**
 * Represents the various possible settings keys.
 */
enum DSKey {
    DS_TIMEZONE,
    DS_BRIGHTNESS,
    DS_HOUR_COLOR,
    DS_MINUTE_COLOR,
    DS_SECOND_COLOR,
    DS_LAST_TIMEZONE_OFFSET,

    DS_NUM_KEYS
};

/**
 * Manages the system settings and provides an interface for changing them.
 */
class DataStoreClass {
    
public:

    /**
     * Initializes the store.
     */
    void begin();

    /**
     * Function signature for an observer function.
     */
    typedef void (*ObserverFunction)(DSKey, int);

    /**
     * Registers a function to be called when the given key's value is updated.
     *
     * @param      key       The setting to observe
     * @param[in]  observer  The function to be called. See above.
     */
    void registerObserver(DSKey key, ObserverFunction observer);

    /**
     * Unregisters all instances of an observer from a key.
     *
     * @param      key       The setting being observed
     * @param[in]  observer  The function to unregister.
     */
    void unregisterObserver(DSKey key, ObserverFunction observer);

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
     */
    void set(DSKey key, int value);

private:

    int store[DS_NUM_KEYS];
    std::vector<ObserverFunction> observers[DS_NUM_KEYS];
};

extern DataStoreClass DataStore;
