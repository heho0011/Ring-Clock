#include "DataStore.h"
#include <EEPROM.h>

#define MAGIC_ADDRESS       511
#define MAGIC_NUMBER        0x42

#define RED                 0xff0000
#define GREEN               0x00ff00
#define BLUE                0x0000ff

void DataStoreClass::begin() {

    // Initialize the store to default values
    memset(&store, 0, sizeof(store));
    store[DS_HOUR_COLOR] = BLUE;
    store[DS_MINUTE_COLOR] = GREEN;
    store[DS_SECOND_COLOR] = RED;

    EEPROM.begin(512);
    
    if (EEPROM.read(MAGIC_ADDRESS) != MAGIC_NUMBER) {

        // EEPROM has never been written
        // Set the EEPROM to the default store values
        EEPROM.put(0, store);

        // Mark it as written
        EEPROM.write(MAGIC_ADDRESS, MAGIC_NUMBER);

        EEPROM.commit();

    } else {
        // Get the saved values from EEPROM
        EEPROM.get(0, store);
    }
}

void DataStoreClass::registerObserver(DSKey key, ObserverFunction observer) {

    observers[key].push_back(observer);
}

void DataStoreClass::unregisterObserver(DSKey key, ObserverFunction observer) {

    for (auto it = observers[key].begin(); it != observers[key].end(); it++) {
        if (*it == observer) {
            observers[key].erase(it);
        }
    }
}

void DataStoreClass::registerValidator(DSKey key, ValidatorFunction validator) {

    validators[key].push_back(validator);
}

void DataStoreClass::unregisterValidator(DSKey key, ValidatorFunction validator) {

    for (auto it = validators[key].begin(); it != validators[key].end(); it++) {
        if (*it == validator) {
            validators[key].erase(it);
        }
    }
}

int DataStoreClass::get(DSKey key) {

    return store[key];
}

bool DataStoreClass::set(DSKey key, int value) {

    // Ignore unchanged values
    if (value == store[key]) {
        return true;
    }

    // Validate the new value
    for (auto& it : validators[key]) {

        if (!(it)(key, value)) {
            // Value was rejected
            return false;
        }
    }

    // All validators accepted. Commit the new value.
    store[key] = value;
    
    EEPROM.put(key * sizeof(int), value);
    EEPROM.commit();

    // Notify the observers
    for (auto& it : observers[key]) {
        (it)(key, value);
    }

    return true;
}

DataStoreClass DataStore;
