#include "ThreePlaceWaitStation.h"


ThreePlaceWaitStation::ThreePlaceWaitStation()
{
}


ThreePlaceWaitStation::~ThreePlaceWaitStation()
{
}

int16_t ThreePlaceWaitStation::stationInit(void){
	int16_t rc = 0;
	uint16_t k;

	/*create Semaphores*/
	stationContr.count = xSemaphoreCreateCounting(MAX_RESOURCES, MAX_RESOURCES);
	if (stationContr.count == NULL) rc = -1;
	stationContr.guard = xSemaphoreCreateMutex();
	if (stationContr.count == NULL) rc = -1;

	/*init Resources*/
	for (k = 0; k < MAX_RESOURCES; k++){
		stationContr.avail[k] = FREE;
	}
	return rc;
}

int16_t ThreePlaceWaitStation::stationReserve(void){
	BaseType_t rc;
	int16_t retcode = 0;
	int16_t k;

	/*request station*/
	rc = xSemaphoreTake(stationContr.count, portMAX_DELAY);
	if (rc == pdFALSE) retcode = -1;
	else{
		/*enter critical section*/
		rc = xSemaphoreTake(stationContr.guard, portMAX_DELAY);
		if (rc == pdFALSE) retcode = -1;
		else{
			/*look for available resource and occupy*/
			k = 0;
			while (k< MAX_RESOURCES && stationContr.avail[k] == OCCUPIED)
			{
				k++;
			}
			if (k >=  MAX_RESOURCES) retcode = -2;
			else{
				retcode = k;
				stationContr.avail[k] = OCCUPIED;
			}
		}
		rc = xSemaphoreGive(stationContr.guard);
		if (rc == pdFALSE) retcode = -1;
	}
	return retcode;
}

int16_t ThreePlaceWaitStation::stationRelease(int16_t which){
	int16_t rc;

	if (which >= 0 && which <  MAX_RESOURCES){
		stationContr.avail[which] = FREE;
		rc = xSemaphoreGive(stationContr.count);
	}
	else rc = -1;
	return rc;
}