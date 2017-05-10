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

volatile sig_atomic_t exitFlag = 0;

void sigintHandler(int) {
    exitFlag = 1;
}

using namespace std;

/*
 * 
 */
int main(int argc, char** argv) {
    if (wiringPiSetup () < 0) {
        cerr << "wiringPi setup error" << endl;
        return 1;
    }
    
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
    
    // Register SIGINT handler
    signal(SIGINT, sigintHandler);
    
    if (wiringPiSetup () < 0) {
        cerr << "wiringPi setup error" << endl;
        return 1;
    }

    // Create sensor object array
    VL53L0X* sensors[SENSOR_COUNT];

    // Create sensors and set pin mode
    for(int i = 0; !exitFlag && i < SENSOR_COUNT; ++i) {
        pinMode(pins[i], OUTPUT);
        digitalWrite(pins[i], LOW);
        sensors[i] = new VL53L0X(pins[i], VL53L0X_ADDRESS_DEFAULT);
        sensors[i]->powerOff();
    }
    
    if(exitFlag) {
        return 0;
    }
    
    // Set up each sensor
    for(int i = 0; !exitFlag && i < SENSOR_COUNT; ++i) {
        digitalWrite(pins[i], HIGH);
        try {
            sensors[i]->init(false);
        } catch(string &err) {
            cerr << err;
        }
        
        sensors[i]->setTimeout(200);
        
        // Set lowest timing budget
        sensors[i]->setMeasurementTimingBudget(20000);
        sensors[i]->setAddress(addresses[i]);
        
        cout << "Sensor " << i << " initialized, real time budget: "
                << sensors[i]->getMeasurementTimingBudget() << endl;
    }
    
    // Start continuous measurement
    for(int i = 0; !exitFlag && i < SENSOR_COUNT; ++i) {
        sensors[i]->startContinuous();
    }
    
    // Durations in nanoseconds
    uint64_t totalDuration = 0;
    uint64_t maxDuration = 0;
    uint64_t minDuration = 1000*1000*1000;
    
    // Initialize reference time measurement
    chrono::steady_clock::time_point t1 = chrono::steady_clock::now();
    
    int j = 0;
    for(; !exitFlag && j < 100000; ++j) {
        cout << "\rReading" << setw(4) << setfill('0') << j << " | ";
        for(int i = 0; !exitFlag && i < SENSOR_COUNT; ++i) {
            uint16_t distance;
            try {
                distance = sensors[i]->readRangeContinuousMillimeters();
            } catch(string &err) {
                cerr << err;
                return 1;
            }
            
            if(sensors[i]->timeoutOccurred()) {
                cout << "\n timeout: " << i << endl;
            } else {
                cout << setw(4) << distance << " | ";
            }
        }
        
        cout << endl;
        
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
    
    // Clean up: delete objects, set GPIO/XSHUT pins to low.
    for(int i = 0; i < SENSOR_COUNT; ++i) {
        sensors[i]->stopContinuous();
        delete sensors[i];
        digitalWrite(pins[i], LOW);
    }

    return 0;
}

