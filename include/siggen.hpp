#pragma once

#include <iostream>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <vector>

#include "ao/ao.h"

#define BUF_SIZE 4096

ao_device *device = 0;
ao_sample_format format;
int driver = 0;

std::vector<int16_t> newBuffer;