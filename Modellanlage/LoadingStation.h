#pragma once
#include "FreeRTOS.h"
#include "WaitStation.h"
#include "semphr.h"

#define Max_RESOURCES 2

class LoadingStation
{
public:
	LoadingStation();
	~LoadingStation();

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

	/*Funktion zum Überprüfen der Strecke*/
	bool getRoadwayStat(void);

	/*Funktion zum Belegen der Strecke*/
	void setRoadwayStat(bool stat);

	/*Funktion zum Reservieren eines Ladeplatzes*/
	int16_t loadPlaceReserve(void);

	/*Funktion zum Reservieren eines Ladeplatzes*/
	int16_t loadPlaceRelease(int16_t which);

private:
	struct poolControl
	{
		SemaphoreHandle_t guard;
		SemaphoreHandle_t count;
		enum state avail[Max_RESOURCES];
	} LoadContr;

	//Variable für Zufahrt Beladestation
	bool roadwayFree = true;
	//Variable Anzahl Beladeplätze
	enum state place[Max_RESOURCES];
};

