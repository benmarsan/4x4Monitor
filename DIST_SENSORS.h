/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   DIST_SENSORS.h
 * Author: bmarsan
 *
 * Created on May 10, 2017, 8:39 PM
 */

#include <VL53L0X.h>

#ifndef DIST_SENSORS_H
#define DIST_SENSORS_H

namespace DIST_SENSORS {
    // Configuration constants
    // Number of sensors. If changed, make sure to adjust pins and addresses accordingly (ie to match size).
    const int SENSOR_COUNT = 2;
    // GPIO pins to use for sensors' XSHUT. As exported by WiringPi.
    const uint8_t pins[SENSOR_COUNT] = { 0, 1 };
    // Sensors' addresses that will be set and used. These have to be unique.
    const uint8_t addresses[SENSOR_COUNT] = {
        VL53L0X_ADDRESS_DEFAULT + 2,
        VL53L0X_ADDRESS_DEFAULT + 4
    };
}

#endif /* DIST_SENSORS_H */

