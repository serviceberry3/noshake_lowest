#ifndef UTILS_H
#define UTILS_H

#include <math.h>

float rangeValue(float value, float min, float max);

void lowPassFilter(float* input, float* output, float alpha);

float fixNanOrInfinite(float value);

#endif