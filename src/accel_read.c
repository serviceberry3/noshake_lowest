#include <stdio.h>
#include <android/looper.h>
#include <android/sensor.h>
#include "drm_low.h"

int main(int argv, char *argc[] ){
    printf("Welcome to the accelerometer reading program, running drm_setup()...\n");
	int ret = drm_setup();

	//create instance of ASensorManager
	ASensorManager *sensorManager = ASensorManager_getInstance();

	//get list of all sensors
	ASensorList sensorList;
	int numSensors = ASensorManager_getSensorList(sensorManager, &sensorList);

	//iterate over all sensors found and print names	
    printf("numSensors is %d\n", numSensors);
	for (int count = 0; count < numSensors; count++) {
		printf("Name :%s, mindelay %d\n", ASensor_getName(sensorList[count]), ASensor_getMinDelay(sensorList[count]));
		//printf("Vendor :%s\n", ASensor_getVendor(sensorList[count]));
		//printf("Type :%s\n", ASensor_getStringType(sensorList[count]));
	}

	//create a sensor eventqueue
	int looperId = 1;
	ASensorEventQueue* eventQueue = ASensorManager_createEventQueue(sensorManager, ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS), looperId, NULL, NULL);

	if (eventQueue == NULL) {
		printf("error, cannot create event queue\n");
		return -1;
	}

	//turn on all available sensors
	for (int count = 0; count < 1; count++) {
		if (ASensorEventQueue_enableSensor(eventQueue, sensorList[count]) != 0)
			printf("error, cannot enable sensor %s\n", ASensor_getName(sensorList[count]));
		else
			printf("Sucessfully enabled sensor %s\n", ASensor_getName(sensorList[count]));

		//set delivery delay rate in microseconds for the accelerometer
		ASensorEventQueue_setEventRate(eventQueue, sensorList[count], 10000);
	}

	//get events from sensors
	int numEvents = 1; //number of events to read before returning

	//poll for some milliseconds before returning
	int kTimeoutMilliSecs = 10000;

	//memory to store incoming sensor event
	ASensorEvent events;
	int identity = 0;


	//poll for events repeatedly
	while ((identity = ALooper_pollAll(-1, NULL, NULL, NULL)) >= 0) {
		//printf("Iteration %d\n", count);

		//clear memory at '''events''' to 0
		memset(&events, 0, sizeof(events));

		//if pollAll timed out, go to next iteration
		if (identity != looperId) {
			printf("pollAll() returned something other than looper ID, probably timed out\n");
			continue;
		}

		//otherwise a sensor has data, so process it now

		//retrieve events. If there's no pending event, go to next iteration
		if (ASensorEventQueue_getEvents(eventQueue, &events, 1) < 1) {
			printf("No pending sensor event\n");
			continue;
		}

		//if it's an accelerometer event, print the data
		if (events.type == ASENSOR_TYPE_ACCELEROMETER) {
			printf("[Accelerometer] x = %f  y = %f  z = %f\n", events.acceleration.x, events.acceleration.y, events.acceleration.z);
		}
	}


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