/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   distanceSensors.cpp
 * Author: bmarsan
 * 
 * Created on May 11, 2017, 9:23 PM
 */

#include "distanceSensors.h"
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <VL53L0X.h>
#include <string>
#include <iostream>

constexpr int distanceSensors::SENSOR_COUNT;
const uint8_t distanceSensors::pins[] = { 0, 1 };
const uint8_t distanceSensors::addresses[] = {
        VL53L0X_ADDRESS_DEFAULT + 2,
        VL53L0X_ADDRESS_DEFAULT + 4
    };
    
distanceSensors::distanceSensors() {
    if (wiringPiSetup () < 0) {
        std::cerr << "wiringPi setup error" << std::endl;
        return;
    }
    
    // Create sensors and set pin mode
    for(int i = 0; i < SENSOR_COUNT; ++i) {
        pinMode(pins[i], OUTPUT);
        digitalWrite(pins[i], LOW);
        sensors[i] = new VL53L0X(pins[i], VL53L0X_ADDRESS_DEFAULT);
        sensors[i]->powerOff();
    }
    
    // Set up each sensor
    for(int i = 0; i < SENSOR_COUNT; ++i) {
        digitalWrite(pins[i], HIGH);
        try {
            sensors[i]->init(false);
        } catch(std::string &err) {
            std::cerr << err;
        }
        
        sensors[i]->setTimeout(200);
        
        // Set lowest timing budget
        sensors[i]->setMeasurementTimingBudget(20000);
        sensors[i]->setAddress(addresses[i]);
        
        std::cout << "Sensor " << i << " initialized, real time budget: "
                << sensors[i]->getMeasurementTimingBudget() << std::endl;
    }
}

distanceSensors::distanceSensors(const distanceSensors& orig) {
}

distanceSensors::~distanceSensors() {
}

void distanceSensors::start() {
    // Start continuous measurement
    for(int i = 0; i < SENSOR_COUNT; ++i) {
        sensors[i]->startContinuous();
    }
}

void distanceSensors::stop() {
    // Start continuous measurement
    for(int i = 0; i < SENSOR_COUNT; ++i) {
        sensors[i]->stopContinuous();
    }
}

uint16_t distanceSensors::readSensorN(int n) {
    
    if(n < SENSOR_COUNT) {
        uint16_t distance;
        
        try {
            distance = sensors[n]->readRangeContinuousMillimeters();
        } catch(std::string &err) {
            std::cerr << err;
            return 1;
        }

        if(sensors[n]->timeoutOccurred()) {
            std::cout << "distanceSensors::readSensorN() timeout (n=" 
                    << n << ")" << std::endl;
            return -1;
        } else {
            return distance;
        }
    } else {
        std::cerr << "distanceSensors::readSensorN() index out of bounds (n="
                << n << ")" << std::endl;
        return -1;
    }
}

void distanceSensors::cleanup() {
    stop();
    // Clean up: delete objects, set GPIO/XSHUT pins to low.
    for(int i = 0; i < SENSOR_COUNT; ++i) {
        digitalWrite(pins[i], LOW);
    }
}