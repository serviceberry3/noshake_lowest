#include "accel_read.h"

//the sensor eventQueue
ASensorEventQueue* eventQueue;
int identity = 0;

//looper ID should always be 1
int looperId = 1;

//memory to store incoming sensor event
ASensorEvent* event;

//number of sensors found
int numSensors;

//the list of sensors
ASensorList sensorList;

//our sensorManager for the program
ASensorManager* sensorManager;

int setup_accelerometer() {
	//create instance of ASensorManager
	sensorManager = ASensorManager_getInstance();

	//allocate memory to hold each incoming event
	event = malloc(sizeof(ASensorEvent));

	//get list of all sensors and store it in global var for all functions to see
	numSensors = ASensorManager_getSensorList(sensorManager, &sensorList);

	//iterate over all sensors found and print names	
    printf("We found %d sensors\n", numSensors);
	if (numSensors == 0)
		return -1;

	for (int count = 0; count < numSensors; count++) {
		printf("Name: %s, mindelay: %d\n", ASensor_getName(sensorList[count]), ASensor_getMinDelay(sensorList[count]));
		//printf("Vendor :%s\n", ASensor_getVendor(sensorList[count]));
		//printf("Type :%s\n", ASensor_getStringType(sensorList[count]));
	}

	//create a sensor eventqueue
	eventQueue = ASensorManager_createEventQueue(sensorManager, ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS), looperId, NULL, NULL);

	if (eventQueue == NULL) {
		printf("error, cannot create event queue\n");
		return -1;
	}

	//turn on accelerometer
	for (int count = 0; count < 1; count++) {
		if (ASensorEventQueue_enableSensor(eventQueue, sensorList[count]) != 0) {
			printf("error, cannot enable sensor %s\n", ASensor_getName(sensorList[count]));
			return -1;
		}
		else
			printf("Sucessfully enabled sensor %s\n", ASensor_getName(sensorList[count]));

		//set delivery delay rate in microseconds for the accelerometer
		ASensorEventQueue_setEventRate(eventQueue, sensorList[count], 10000);
	}

	return 0;
}

ASensorEvent* read_accelerometer() {
	//poll for events, timeout -1 means wait forever until got data
	if ((identity = ALooper_pollAll(-1, NULL, NULL, NULL)) >= 0) {
		//printf("Iteration %d\n", count);

		//clear memory at '''events''' to 0
		memset(event, 0, sizeof(ASensorEvent));

		//if pollAll timed out, go to next iteration
		if (identity != looperId) {
			printf("pollAll() returned something other than looper ID, probably timed out\n");
			return NULL;
		}

		//otherwise a sensor has data, so process it now

		//retrieve events. If there's no pending event, go to next iteration
		if (ASensorEventQueue_getEvents(eventQueue, event, 1) < 1) {
			printf("No pending sensor event\n");
			return NULL;
		}

		//if it's an accelerometer event, print the data and return it
		if (event->type == ASENSOR_TYPE_ACCELEROMETER) {
			//printf("[Accelerometer] x = %f  y = %f  z = %f\n", event->acceleration.x, event->acceleration.y, event->acceleration.z);
			return event;
		}
	}

	return NULL;
}

int close_accelerometer() {
	//turn off all available sensors
	for (int count = 0; count < numSensors; count++) {
		if (ASensorEventQueue_disableSensor(eventQueue, sensorList[count]) != 0) {
			printf("ERROR: cannot disable sensor %s\n", ASensor_getName(sensorList[count]));
		}
	}

	//free resources
	ASensorManager_destroyEventQueue(sensorManager, eventQueue);

	return 0;
}