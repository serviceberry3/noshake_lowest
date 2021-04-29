#include "utils.h"


float rangeValue(float value, float min, float max)
{
    //apply boundaries to a given value
    if (value > max) return max;

    //otherwise if val less than min return min, otherwise return value
    return fmax(value, min);
}


void lowPassFilter(float* input, float* output, float alpha)
{
    //iterate through all three elements of the input float array
    for (int i = 0; i < 3; i++) {
        //set that slot in the output array to its previous value plus alphaConstant * (change in the value since last reading)
        output[i] = output[i] + (alpha * (input[i] - output[i])); //we only allow the acceleration reading to change by 85% of its actual change

        //a second way to implement
        //output[i] = input[i] - (alpha * output[i] + (1-alpha) * input[i]);
    }
}

//TODO: filter to change a NaN or infinite value to 0
float fixNanOrInfinite(float value)
{
    return 0;
}