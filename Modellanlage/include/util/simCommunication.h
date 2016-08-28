	/**************************************************************************************************
	Module:		simCommunication
	Purpose:	Stellt das Kommunikationsinterface zum Modellbahn Simulator bereit
	System:		Visual Studio / Windows - FreeRTOS Simulator
	Date:		21/05/2014
**************************************************************************************************/
#ifndef SIMCOMMUNICATION_H
#define SIMCOMMUNICATION_H
#ifdef __cplusplus
extern "C"
{
#endif
#ifdef ENABLE_SIMCOM
#include "semphr.h"
/*	initSystem
	Zweck:				Aufbau der Kommunikation mit dem Zielsystem
	Randbedingungen:	Diese Funktion muss aus einem FreeRTOS Task aufgerufen werden
						Zielsystem muss gestartet sein und noch keine aktive Verbindung haben
						Dieser Aufruf ist blockierend und kehrt erst zurück wenn eine Bestätigung empfangen wurde
	Parameter:			keine
	Rückgabe:			xQueueHandle - Elementgröße: int
	Bedeutungen:
	Sensoren:
	0  - Einfahrt Startbereich
	1  - Ausfahrt Startbereich	
	2  - Einfahrt Beladestation 1
	3  - Ausfahrt Beladestation 1
	4  - Einfahrt Beladestation 2
	5  - Ausfahrt Beladestation 2
	6  - Einfahrt Waage in Richtung Entladestation
	7  - Ausfahrt Waage in Richtung Entladestation
	8  - Einfahrt Entladestation 1
	9  - Ausfahrt Entladestation 1
	10 - Einfahrt Entladestation 2
	11 - Ausfahrt Entladestation 2
	12 - Einfahrt Entladestation 3
	13 - Ausfahrt Entladestation 3
	14 - Einfahrt Wartebereich
	15 - Ausfahrt Wartebereich
	16 - Einfahrt Waage in Richtung Beladestation
	17 - Ausfahrt Waage in Richtung Beladestation

	Aktivitäten:
	20 - Wiegen abgeschlossen
	21 - Beladen Platz 1 abgeschlossen
	22 - Beladen Platz 2 abgeschlossen
	23 - Entladen Platz 1 abgeschlossen
	24 - Entladen Platz 2 abgeschlossen
	25 - Entladen Platz 3 abgeschlossen

	Sonstige:
	30 - Startfreigabe erteilt
	31 - Endsequenz eingeleitet
*/	
extern xQueueHandle initSystem(void);

/*	shutdownSystem
	Zweck:				Schließt eine offene Verbindung
	Randbedingungen:	Eine Verbindung muss geöffnet worden sein
	Parameter:			keine						
*/
extern void shutdownSystem(void);

/*	sendTo
	Zweck:				Sendet einen Befehl an die Simulation
	Randbedingunen:		initSystem muss erfolgreich aufgerufen worden sein
	Parameter:			
			int who [in]
						0  - Stoppstelle Startbereich
						1  - Stoppstelle Beladestation 1
						2  - Stoppstelle Beladestation 2
						3  - Stoppstelle Waage
						4  - Stoppstelle Entladestation 1
						5  - Stoppstelle Entladestation 2
						6  - Stoppstelle Entladestation 3
						7  - Stoppstelle Wartebereich

						10 - Weiche Beladestation
						11 - Weiche Entladestation 1/2
						12 - Weiche Entladestation 3

						20 - Wiegen starten
						21 - Beladen auf Platz 1 starten
						22 - Beladen auf Platz 2 starten
						23 - Entladen auf Platz 1 starten
						24 - Entladen auf Platz 2 starten
						25 - Entladen auf Platz 3 starten

			int command [in]
				Stoppstelle:
						0 - deaktivieren
						1 - aktivieren


				Weiche:
					Beladestation:
						1 - Station 1
						2 - Station 2

					Entladestation 1/2:
						1 - Entladestation 1
						2 - Durchfahrt
						3 - Entladestation 2

					Entladestation 3:
						1 - Entladestation 3
						2 - Durchfahrt


				Aktivitäten:
						Wird ignoriert

	Rückgabe:
						< 0: Fehler
						> 0: Anzahl der Versendeten Bytes
*/
extern int sendTo(int who, int command);

#endif
#ifdef __cplusplus
}
#endif
#endif