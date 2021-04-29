#ifndef IMPULSE_RESP_H
#define IMPULSE_RESP_H

#include <stdlib.h>
#include <math.h>
#include <assert.h>

#define HZ 211
#define TAU 0

typedef struct imp_resp_info {
    //actual data
    float* resp_array;
    size_t size;
    float eValue;
    float kValue;
} imp_resp_info;

imp_resp_info* imp_resp_arr_create(size_t sz, float e, float k);

void imp_resp_arr_destroy(imp_resp_info* info);

void imp_resp_arr_populate(imp_resp_info* info);

float imp_resp_arr_get_value(imp_resp_info*, int index);

float imp_resp_arr_get_sum(imp_resp_info* info);

#endif