#include "LoadingStation.h"

LoadingStation::LoadingStation()
{
	place[0] = FREE;
	place[1] = FREE;
}


LoadingStation::~LoadingStation()
{
}

int16_t LoadingStation::stationInit(void){
	int16_t rc = 0;
	uint16_t k;

	/*create Semaphores*/
	LoadContr.count = xSemaphoreCreateCounting(Max_RESOURCES, Max_RESOURCES);
	if (LoadContr.count == NULL) rc = -1;
	LoadContr.guard = xSemaphoreCreateMutex();
	if (LoadContr.count == NULL) rc = -1;

	/*init Resources*/
	for (k = 0; k < Max_RESOURCES; k++){
		LoadContr.avail[k] = FREE;
	}
	return rc;
}

int16_t LoadingStation::stationReserve(void){	
	BaseType_t rc;
	int16_t retcode = 0;
	int16_t k;

	/*request station*/
	rc = xSemaphoreTake(LoadContr.count, portMAX_DELAY);
	if (rc == pdFALSE) retcode = -1;
	else{
		/*enter critical section*/
		rc = xSemaphoreTake(LoadContr.guard, portMAX_DELAY);
		if (rc == pdFALSE) retcode = -1;
		else{
			/*look for available resource and occupy*/
			k = 0;
			while (k<Max_RESOURCES && LoadContr.avail[k] == OCCUPIED)
			{
				k++;
			}
			if (k >= Max_RESOURCES) retcode = -2;
			else{
				retcode = k;
				LoadContr.avail[k] = OCCUPIED;
			}
		}
		rc = xSemaphoreGive(LoadContr.guard);
		if (rc == pdFALSE) retcode = -1;
	}
	return retcode;
}

int16_t LoadingStation::stationRelease(int16_t which){
	int16_t rc;

	if (which >= 0 && which < Max_RESOURCES){
		LoadContr.avail[which] = FREE;
		rc = xSemaphoreGive(LoadContr.count);
	}
	else rc = -1;
	return rc;
}

void LoadingStation::setRoadwayStat(bool stat){
	roadwayFree = stat;
}

bool LoadingStation::getRoadwayStat(void){
	return roadwayFree;
}

int16_t LoadingStation::loadPlaceReserve(void){
	int16_t retcode = -1;
	int16_t k = 0;

	while (k < Max_RESOURCES && place[k] == OCCUPIED){
		k++;
	}

	if (k >= Max_RESOURCES) retcode = -2;
	else {
		retcode = k;
		place[k] = OCCUPIED;	
	}
	return retcode;
}

int16_t LoadingStation::loadPlaceRelease(int16_t which){
	int16_t errorCode = 0;

	if (which >= 0 && which < Max_RESOURCES){
		place[which] = FREE;
		errorCode = 0;
	}
	else errorCode = -1;
	return errorCode;
}