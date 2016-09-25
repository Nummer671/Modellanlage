#pragma once
#include "FreeRTOS.h"
#include "WaitStation.h"
#include "semphr.h"

#define MAX_RESOURCES 3

class ThreePlaceWaitStation
{
public:
	ThreePlaceWaitStation();
	~ThreePlaceWaitStation();

	/*Funktion zum Initialisieren der Station
	Bei Erfolg gibt die Station eine 0 zurück,
	ansonsten -1*/
	int16_t stationInit(void);

	/*Funktion zum Reservieren eines Stellplatzes.
	Bei Erfolg wird die Nummer des Stellplatzes zurückgegeben,
	ansonsten gibt die Funktion -1 zurück*/
	int16_t stationReserve(void);

	/*Funktion zum Freigeben eines Stellplatzes.
	Die Funktion bekommt die Nummer des Stellplatzes übergeben.
	Bei einem Fehler gibt die Funktion -1 zurück, ansonsten pdPass,
	um anzuzeigen, dass die Semaphore erfolgreich freigegeben wurde*/
	int16_t stationRelease(int16_t which);

	/*Funktion zum Reservieren der Weiche*/
	int16_t switch2Reserve(void);

	/*Funktion zum Freigeben der Weiche*/
	int16_t switch2Release(void);

private:
	struct poolControl
	{
		SemaphoreHandle_t guard;
		SemaphoreHandle_t count;
		enum state avail[MAX_RESOURCES];
	} stationContr;

	struct switchControl{
		SemaphoreHandle_t guard;
		enum state avail;
	} switch2;
};

