#include "convolve.h"



convolver* convolver_create(circ_buf* data, imp_resp_info* imp_resp) {
    convolver* new_convolver = malloc(sizeof(convolver));

    new_convolver->xArray = data->buf;

    //TODO: make this value dynamic
    new_convolver->xLength = 211;

    new_convolver->hArray = imp_resp->resp_array;
    new_convolver->hLength = (int)imp_resp->size;

    new_convolver->yLength = new_convolver->xLength + new_convolver->hLength - 1;

    //allocated appropriately sized float array for the output signal (m + n - 1), set entire array to 0 to begin accumulation
    new_convolver->yArray = (float*) calloc(sizeof(float), new_convolver->yLength);

    new_convolver->tempXArray = NULL;

    return new_convolver;
}

void convolver_destroy(convolver* convolver) {
    if (convolver->tempXArray != NULL) free(convolver->tempXArray);
    free(convolver->yArray);

    free(convolver);
}

float convolver_convolve(convolver* convolver, int curr_head) {
    /*
    float* tempHArray = (float*) calloc(yLength, sizeof(float));
    memcpy(tempHArray, hArray, hLength);
    */
    //set up a temporary array of floats
    convolver->tempXArray = (float*) malloc(sizeof(float) * convolver->xLength);


    //we want to order the data from the circular buffer from oldest to newest, using the head as the break point
    memcpy(convolver->tempXArray, convolver->xArray + curr_head, sizeof(float) * (convolver->xLength - curr_head));
    memcpy(convolver->tempXArray + (convolver->xLength - curr_head), convolver->xArray, sizeof(float) * curr_head);

    //perform convolution operation
    for (int i = 0; i < convolver->xLength; i++)
    {
        for (int j = 0; j < convolver->hLength; j++)
        {
            convolver->yArray[i + j] += convolver->tempXArray[i] * convolver->hArray[j];
        }
    }

    return convolver->tempXArray[84];
}

float convolver_getTempXMember(convolver* convolver, int index) {
    if (convolver->tempXArray == NULL)
        return -1;

    return convolver->tempXArray[index];
}

float convolver_getYMember(convolver* convolver, int index) {
    return convolver->yArray[index];
}

float convolver_getHMember(convolver* convolver, int index) {
    return convolver->hArray[index];
}

float convolver_getXMember(convolver* convolver, int index) {
    return convolver->xArray[index];
}

size_t convolver_getYSize(convolver* convolver) {
    return convolver->yLength;
}