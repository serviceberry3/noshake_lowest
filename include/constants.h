#ifndef CONSTANTS_H
#define CONSTANTS_H

//constants for NoShake
#define MAX_ACC (float) 8.0
#define SPRING_CONST (float) 400.0
#define DAMPENER_FRIX_CONST (float) (2.0 * Math.sqrt(SPRING_CONST))
#define LOW_PASS_ALPHA (float) 0.9 //0.9
#define Y_FACTOR (float) 2000 //650 //latest: 1200
#define E (float) 2.71828
#define SHAKING_THRESH (float) 0.1
#define BUFF_SZ 211
#define EXTRA_FRIX_CONST (float) 0.1
#define POSITION_FRICTION_DEFAULT (float) 0.1f

#endif
