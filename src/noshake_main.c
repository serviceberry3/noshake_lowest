#include "accel_read.h"
#include "circ_buffer.h"
#include "impulse_resp.h"
#include "convolve.h"
#include "constants.h"
#include "utils.h"

//whether to apply the NoShake correction or leave text/square stagnant
#define APPLY_CORRECTION true

float StempAcc[3];
float Sacc[3];
float accAfterFrix[3];

//long to use for keeping track of thyme
long timestamp = 0;

//changes in x and y to be used to move the draggable text based on user's finger
int _xDelta, _yDelta;

//time variables, and the results of H(t) and Y(t) functions
double HofT, YofT, startTime, timeElapsed;

//the raw values that the low-pass filter is applied to
float gravity[3];

//working on circular buffer for the data
float accelBuffer[3];

float impulseSum;

//is the device shaking?
volatile int shaking = 0;

//should be init to 0
int indx = 0, check = 0, times = 0;

//proper x and y adjustments for this cycle
float toMoveX, toMoveY;

//circular buffers to hold latest x and y acceleration data
circ_buf* x_buf;
circ_buf* y_buf;

//the impulse response data (static)
imp_resp_info* imp_resp_data;

//convolvers for x and y accel buffers
convolver* y_signal_convolver_x;
convolver* y_signal_convolver_y;

//all processing of accelerometer event data for NoShake takes place here
int noShakeZhongLin(ASensorEvent* event) {
    //avoid processing excessive accelerations
    StempAcc[0] = rangeValue(event->acceleration.x, MAX_ACC, MAX_ACC);
    StempAcc[1] = rangeValue(event->acceleration.y, MAX_ACC, MAX_ACC);

    //apply the low pass filter to reduce noise
    lowPassFilter(StempAcc, Sacc, LOW_PASS_ALPHA);

    //try to eliminate noise by knocking low acceleration values down to 0 (also make text re-center faster)
    if (fabs(Sacc[0]) <= 0.5) {
        Sacc[0] = 0;
    }
    if (fabs(Sacc[1]) <= 0.5) {
        Sacc[1] = 0;
    }

    //apply some extra friction (hope is to make text return to center of screen a little faster)
    //rapid decreases will be highlighted by this
    float xFrixToApply = accAfterFrix[0] * EXTRA_FRIX_CONST;
    float yFrixToApply = accAfterFrix[1] * EXTRA_FRIX_CONST;

    //apply the friction to get new x and y acceleration values
    accAfterFrix[0] = Sacc[0] - xFrixToApply;
    accAfterFrix[1] = Sacc[1] - yFrixToApply;

    //add the x and y acceleration values to the circular buffer
    int h = circ_buf_put(x_buf, accAfterFrix[0]);
    int l = circ_buf_put(y_buf, accAfterFrix[1]);

    //printf("Successfully ran noShakeZhongLin()\n");

    //convolve the circular buffer of acceleration data with the impulse response array to get Y(t) array
    float f = convolver_convolve(y_signal_convolver_x, circ_buf_get_head(x_buf));
    float y = convolver_convolve(y_signal_convolver_y, circ_buf_get_head(y_buf));

    float deltaX = 0;
    float deltaY = 0;

    
    //do calculations
    for (int i = 0; i < BUFF_SZ; i++) {
        float impulseValue = imp_resp_arr_get_value(imp_resp_data, i);
        deltaX += impulseValue * convolver_getTempXMember(y_signal_convolver_x, i);
        deltaY += impulseValue * convolver_getTempXMember(y_signal_convolver_y, i);
    }

    //normalize the scale of filters with arbitrary length/magnitude
    deltaX /= impulseSum;
    deltaY /= impulseSum;

    //calculate how much we need to move text in Y direction for this frame
    toMoveY = -1 *                                                              //flip
            (deltaX - POSITION_FRICTION_DEFAULT * deltaX)        //reduce deltaX by adding some friction. Use deltaX for Y displacement b/c screen is horizontal
            * Y_FACTOR;                                         //arbitrary scaling factor


    //calculate how much we needto move text in x direction for this frame
    toMoveX = -1 *                                                               //flip
        (deltaY - POSITION_FRICTION_DEFAULT * deltaY)         //reduce deltaY by adding some friction. Use deltaY for X displacement b/c screen is horizontal
        * Y_FACTOR;                                          //arbitrary scaling factor


    if (APPLY_CORRECTION) {
        //set crtc x and y offsets appropriately
        printf("Corrections to be made are %f on x and %f on y\n", toMoveX, toMoveY);
    }

    return 0;
}


int main(int argv, char* argc[]) {
    //initialize accAfterFrix array to all zeroes
    accAfterFrix[0] = accAfterFrix[1] = accAfterFrix[2] = 0.0;

    gravity[0] = gravity[1] = gravity[2] = 0;
    accelBuffer[0] = accelBuffer[1] = accelBuffer[2] = 0;

    //initialize a circular buffer of 211 floats
    x_buf = circ_buf_create(BUFF_SZ);
    y_buf = circ_buf_create(BUFF_SZ);

    //initialize an impulse response array also of size 211
    imp_resp_data = imp_resp_arr_create(BUFF_SZ, E, SPRING_CONST);

    //populate the H(t) impulse response array in C++ based on the selected spring constant
    imp_resp_arr_populate(imp_resp_data);

    //store sum of filter
    impulseSum = imp_resp_arr_get_sum(imp_resp_data);

    //instantiate a convolver which has a pointer to both the circular buffer and the impulse response array
    y_signal_convolver_x = convolver_create(x_buf, imp_resp_data);
    y_signal_convolver_y = convolver_create(y_buf, imp_resp_data);

    printf("Welcome to main()!\n");

    //set up the accelerometer for reading
    if (setup_accelerometer()) {
        printf("setup_accelerometer() failed, returning from main()...\n");
        return -1;
    }

    //vars to store accelerometer event data
    ASensorEvent* event;
    float x, y, z;

    //main loop of the program
    while (1) {
        //read data from sensor event
        event = read_accelerometer();

        //process data and adjust content
        noShakeZhongLin(event);
    }
}

