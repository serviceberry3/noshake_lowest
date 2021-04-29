#ifndef ACCEL_READ_H
#define ACCEL_READ_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <android/looper.h>
#include <android/sensor.h>

//prepare the accelerometer for reading
int setup_accelerometer();

//read accelerometer data
ASensorEvent* read_accelerometer();

//close the sensor/free resources
int close_accelerometer();

#endif