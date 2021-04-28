#include <stdlib.h>
#include <stdio.h>
#include "circ_buffer.h"
#include "impulse_resp.h"
#include "convolve.h"
#include "constants.h"


int main(int argv, char *argc[]) {
    /*
    //initialize a circular buffer of 211 floats
    CircBuffer.circular_buffer(NoShakeConstants.buffer_size, 0);
    CircBuffer.circular_buffer(NoShakeConstants.buffer_size, 1);

    //initialize an impulse response array also of size 211
    ImpulseResponse.impulse_resp_arr(NoShakeConstants.buffer_size, NoShakeConstants.e, NoShakeConstants.spring_const);

    //populate the H(t) impulse response array in C++ based on the selected spring constant
    ImpulseResponse.impulse_response_arr_populate();

    //store sum of filter
    impulseSum = ImpulseResponse.impulse_response_arr_get_sum();
    Log.d("DBUG", String.format("Impulse sum is %f", impulseSum));

    //instantiate a convolver which has a pointer to both the circular buffer and the impulse response array
    Convolve.convolver(CircBuffer.circular_buf_address(0), 0);
    Convolve.convolver(CircBuffer.circular_buf_address(1), 1);


    gravity[0] = gravity[1] = gravity[2] = 0;
    accelBuffer[0] = accelBuffer[1] = accelBuffer[2] = 0;*/


    //main loop of the program 

    //test a malloc
    void* test_malloc = malloc(500);

    if (test_malloc == NULL) {
        printf("MALLOC TEST FAILED\n");
    }
    else {
        printf("MALLOC TEST SUCCESS\n");
    }
}

