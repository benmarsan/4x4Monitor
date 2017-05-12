/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: bmarsan
 *
 * Created on May 9, 2017, 5:57 PM
 */

#include <cstdlib>
#include <chrono>
#include <csignal>
#include <iomanip>
#include <iostream>
#include <unistd.h>
#include <wiringPi.h>
#include <VL53L0X.h>
#include "tsl2561.h"
#include "DIST_SENSORS.h"
#include "LUX_SENSOR.h"
#include "distanceSensors.h"

volatile sig_atomic_t exitFlag = 0;

void sigintHandler(int) {
    exitFlag = 1;
}

using namespace std;

// VL54L0X distance sensor object array
distanceSensors dist;
// TSL2561 lux sensor
void *tsl;

/*
 * 
 */

int init() {
    // Register SIGINT handler
    signal(SIGINT, sigintHandler);
    
    if (wiringPiSetup () < 0) {
        cerr << "wiringPi setup error" << endl;
        return 1;
    }
    
    dist = distanceSensors();
    
    dist.start();
    
    tsl = tsl2561_init(LUX_SENSOR::address, LUX_SENSOR::i2c_device);
    tsl2561_enable_autogain(tsl);
    tsl2561_set_gain(tsl, TSL2561_INTEGRATION_TIME_13MS);
    
    if(tsl == NULL) {
        cerr << "TSL2561 setup error" << endl;
        return 1;
    }
    
    return 0;
}

/*
 * 
 */
int main(int argc, char** argv) {
    if(init() != 0) {
        cout << "Error in initialization" << endl;
    } else {
        cout << "Initialization completed successfully" << endl;
    }
    
    
    
    // Durations in nanoseconds
    uint64_t totalDuration = 0;
    uint64_t maxDuration = 0;
    uint64_t minDuration = 1000*1000*1000;
    
    // Initialize reference time measurement
    chrono::steady_clock::time_point t1 = chrono::steady_clock::now();
    
    int j = 0;
    for(; !exitFlag && j < 10000; ++j) {
        cout << "\rReading" << setw(4) << setfill('0') << j << " mm | ";
        
        // Read distance sensors
        for(int i = 0; !exitFlag && i < distanceSensors::SENSOR_COUNT; ++i) {
            uint16_t distance = dist.readSensorN(i);
            
            if(distance < 0) {
                cout << "\n timeout: " << i << endl;
            } else {
                cout << setw(4) << distance << " | ";
            }
        }
        
        // Read lux sensor
        long lux;
        lux = tsl2561_lux(tsl);
        cout << setw(4) << lux << " lux | ";
        
        cout << flush;
        
        // Calculate current iteration time
        chrono::steady_clock::time_point t2 = chrono::steady_clock::now();
        uint64_t duration = (chrono::duration_cast<chrono::nanoseconds>(t2 - t1)).count();
        t1 = t2;
        
        // Add to total duration
        totalDuration += duration;
        
        // Nothing to compare on first run
        if(j == 0) {
            continue;
        }
        
        // Save max and min data
        
        if(duration > maxDuration) {
            maxDuration = duration;
        }
        
        if(duration < minDuration) {
            minDuration = duration;
        }        
    }
    
    // Print duration data
    cout << "\nMax duration: " << maxDuration << "ns" << endl;
    cout << "Min duration: " << minDuration << "ns" << endl;
    cout << "Avg duration: " << totalDuration / ( j + 1 ) << "ns" << endl;
    cout << "Avg frequency: " << 1000000000 / (totalDuration / ( j + 1 )) << "Hz" << endl;
    
    dist.cleanup();
    
    tsl2561_close(tsl);
    LUX_SENSOR::i2c_device = NULL;

    return 0;
}

