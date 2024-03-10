#pragma once

#include <Wire.h>

#define MATH_MIN(a,b) ((a)<(b)?(a):(b))
#define MATH_MAX(a,b) ((a)>(b)?(a):(b))

#define PIN_I2C_SDA 2
#define PIN_I2C_SCL 3

#define DEEP_SLEEP_MICROS 5 * 1000 * 1000

