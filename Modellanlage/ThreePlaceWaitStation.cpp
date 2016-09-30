#include "ThreePlaceWaitStation.h"
#include <stdio.h>


ThreePlaceWaitStation::ThreePlaceWaitStation()
{
}


ThreePlaceWaitStation::~ThreePlaceWaitStation()
{
}

int16_t ThreePlaceWaitStation::stationInit(void){
	int16_t rc = 0;
	uint16_t k;

	/*Semaphoren erzeugen*/
	stationControl.count = xSemaphoreCreateCounting(MAX_RESOURCES, MAX_RESOURCES);
	if (stationControl.count == NULL) rc = -1;
	stationControl.guard = xSemaphoreCreateMutex();
	if (stationControl.guard == NULL) rc = -1;

	/*Ressourcen initialisieren*/
	for (k = 0; k < MAX_RESOURCES; k++){
		stationControl.avail[k] = FREE;
	}
	return rc;
}

int16_t ThreePlaceWaitStation::stationReserve(void){
	BaseType_t rc;
	int16_t retcode = 0;
	int16_t k;

	/*Station anfragen*/
	rc = xSemaphoreTake(stationControl.count, portMAX_DELAY);
	if (rc == pdFALSE) retcode = -1;
	else{
		/*kritischen Bereich betreten*/
		rc = xSemaphoreTake(stationControl.guard, portMAX_DELAY);
		if (rc == pdFALSE) retcode = -1;
		else{
			/*nach freien Ressourcen schauen und belegen*/
			k = 0;
			while (k< MAX_RESOURCES && stationControl.avail[k] == OCCUPIED)
			{
				k++;
			}
			if (k >= MAX_RESOURCES) retcode = -2;
			else{
				retcode = k;
				stationControl.avail[k] = OCCUPIED;
			}
		}
		rc = xSemaphoreGive(stationControl.guard);
		if (rc == pdFALSE) retcode = -1;
	}
	return retcode;
}

int16_t ThreePlaceWaitStation::stationRelease(int16_t which){
	int16_t rc;

	if (which >= 0 && which <  MAX_RESOURCES){
		stationControl.avail[which] = FREE;
		rc = xSemaphoreGive(stationControl.count);
	}
	else rc = -1;
	return rc;
}