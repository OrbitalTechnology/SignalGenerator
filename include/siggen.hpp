#pragma once

#include <iostream>
#include <string.h>
#include <math.h>
#include <stdint.h>

#include "ao/ao.h"

#define BUF_SIZE 4096

ao_device *device = 0;
ao_sample_format format;
int driver = 0;

int16_t* buffer = 0;
int buf_size = 0;

int16_t* bufferStop = 0;
int buf_size_stop = 0;