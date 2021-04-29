#include "impulse_resp.h"


imp_resp_info* imp_resp_arr_create(size_t sz, float e, float k) {
    imp_resp_info* new_arr = malloc(sizeof(imp_resp_info));
    new_arr->resp_array = malloc(sizeof(float) * sz);

    new_arr->eValue = e;
    new_arr->kValue = k;
    new_arr->size = sz;

    return new_arr;
}

void imp_resp_arr_destroy(imp_resp_info* info) {
    free(info->resp_array);
    free(info);
}

//fill the array with the appropriate H(t) result based on spring constant k selected
void imp_resp_arr_populate(imp_resp_info* info) {
    //divide 4.0 seconds by the size of the array
    float sqrtK = sqrt(info->kValue);

    /*
    float timeIncrement = 4.0f/size;

    for (int i=0; i<size; i++) {
        float currTime = timeIncrement * i;
        //fill in this spot in the array with the appropriate H(t) value
        responseArray[i] = ((float)(currTime) * pow(eValue, -currTime * sqrtK));
    }
     */

    for (int t = 0; t < info->size; t++) {
        info->resp_array[info->size - 1 - t] = (float)(t + TAU) / HZ * pow(info->eValue, -(float)(t + TAU) / HZ * sqrtK);
    }
}



float imp_resp_arr_get_value(imp_resp_info* info, int index) {
    return info->resp_array[index];
}

float imp_resp_arr_get_sum(imp_resp_info* info) {
    assert(info->resp_array != NULL);
    assert(info->size > 0);

    float sum = 0;

    for (int i = 0; i < info->size; i++) {
        sum += info->resp_array[i];
    }
    return sum;
}