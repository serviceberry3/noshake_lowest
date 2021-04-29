#ifndef CONVOLVE_H
#define CONVOLVE_H

#include <stdlib.h>
#include <string.h>
#include "impulse_resp.h"
#include "circ_buffer.h"

typedef struct convolver {
    float* hArray;
    int hLength;
    float* xArray;
    int xLength;
    float* yArray;
    int yLength;

    float* tempXArray;
} convolver;


convolver* convolver_create(circ_buf* data, imp_resp_info* imp_resp);

void convolver_destroy(convolver* convolver);

float convolver_convolve(convolver* convolver, int curr_head);

float convolver_getYMember(convolver* convolver, int index);

float convolver_getHMember(convolver* convolver, int index);

float convolver_getXMember(convolver* convolver, int index);

float convolver_getTempXMember(convolver* convolver, int index);

size_t convolver_getYSize(convolver* convolver);


#endif