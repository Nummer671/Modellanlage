#pragma once
/***************************************************
Module name: ThreePlaceWaitStation.h
First written on xxxxx by Severin Wagner.
Module Description:
Header Datei für die Klasse ThreePlaceWaiStation.
In der Datei werden die Attribute und Methoden der
Klasse angelegt. Die Beschreibung der jeweiligen
Elemente befindet sich bei diesen.
***************************************************/
/* Include section */
#include "FreeRTOS.h"
#include "WaitStation.h"
#include "semphr.h"
/***************************************************/
/* Defines section */
#define MAX_RESOURCES 3
/***************************************************/
/* Function Prototype Section
Add prototypes for all *** called by this
module, with the exception of runtime routines.
***************************************************/


class ThreePlaceWaitStation
{
public:
	ThreePlaceWaitStation();
	~ThreePlaceWaitStation();


	/**************************************************
	Function name:  int16_t stationInit(void)
	returns:		0:Erfolg; -1:Fehler
	Created by: Severin Wagner
	Date created: date
	Description: Die Funktion initialisiert die Attribute
	und Semaphoren der Klasse.
	Notes: -
	**************************************************/
	int16_t stationInit(void);

	/**************************************************
	Function name:  int16_t stationReserve(void)
	returns: 0:Entladeplatz 1; 1: Entladeplatz 2;
	2: Entladeplatz 3; -1:Fehler
	Created by: Severin Wagner
	Date created: date
	Description: Die Funktion reserviert einen Entlade-
	platz. Ist kein Entladeplatz frei, dann blockiert
	die Methode die aufrufende Task solange bis eine
	Station freigeworden ist.
	Notes: -
	**************************************************/
	int16_t stationReserve(void);


	/**************************************************
	Function name: int16_t stationRelease(int16_t which)
	returns:
	which: Gibt die Nummer der Station an, die frei-
	gegeben werden sollen. Die Nummer kann 0,1 oder 2
	sein.
	Created by: author's name
	Date created: date
	Description: Die Funktion bekommt die Nummer des
	Stellplatzes übergeben. Bei einem Fehler gibt die
	Funktion -1 zurück, ansonsten pdPass, um anzuzeigen,
	dass die Semaphore erfolgreich freigegeben wurde
	Notes: restrictions, odd modes
	**************************************************/
	int16_t stationRelease(int16_t which);


private:
	struct poolControl
	{
		SemaphoreHandle_t guard;
		SemaphoreHandle_t count;
		enum state avail[MAX_RESOURCES];
	} stationControl;
};

