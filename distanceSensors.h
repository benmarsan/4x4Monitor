/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   distanceSensors.h
 * Author: bmarsan
 *
 * Created on May 11, 2017, 9:23 PM
 */

#ifndef DISTANCESENSORS_H
#define DISTANCESENSORS_H

#include <VL53L0X.h>

class distanceSensors {
public:
    static constexpr int SENSOR_COUNT {2};
    static const uint8_t pins[SENSOR_COUNT];
    static const uint8_t addresses[SENSOR_COUNT];
    VL53L0X *sensors[SENSOR_COUNT];
    
    distanceSensors();
    distanceSensors(const distanceSensors& orig);
    virtual ~distanceSensors();
    void start();
    void stop();
    uint16_t readSensorN(int n);
    void cleanup();
    
    
private:
    int fd;
};

#endif /* DISTANCESENSORS_H */

