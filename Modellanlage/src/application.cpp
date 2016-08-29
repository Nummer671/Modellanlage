/**************************************************************************************************
Module:		application.cpp
Purpose:	This template provides a FreeRTOS Windows Simulation
System:		Visual Studio / Windows - FreeRTOS Simulator
Date:		2016-03-11
Version:	2.1.0
**************************************************************************************************/
#include <FreeRTOS.h>
#include <stdio.h>
#include "task.h"
#include "util\bprint.h"
#include "util\simCommunication.h"
#include "ComCodes.h"

using namespace std;

void loadingStationFunc(void* pvParameters);

xQueueHandle statusQueue;

/******* Insert your code in this task **********************************************************/
extern "C" void taskApplication(void *pvParameters)
{
	(void*)pvParameters; // Prevent unused warning

	int myBuffer;
	void* pvMyBuffer = &myBuffer;
	
	const char scale[]			= "scale";
	const char waitStation[]	= "waitStation";
	const char startArea[]		= "startArea";
	const char loadStation[]	= "loadStation";
	const char unloadStation[]	= "unloadStation";
	
	vTaskPrioritySet(NULL, tskIDLE_PRIORITY + 5);

	//Initialisieren des Simulators
	//initSystem() gibt die Message Queue zurück, auf der die Nachrichten des Simulators empfangen werden.
	statusQueue = initSystem();
	xQueueReceive(statusQueue, &myBuffer, portMAX_DELAY);
	if (myBuffer == 30) {
		printf("Simulation erfolgreich gestartet. \n");
		xTaskCreate(loadingStationFunc, loadStation, configMINIMAL_STACK_SIZE, pvMyBuffer, tskIDLE_PRIORITY + 4, NULL);
	}
	else puts("Simulation konnte nicht gestartet werden. \n");
	

	// Start des Steuerprogramms
	while (true)
	{

		xQueueReceive(statusQueue, &myBuffer, portMAX_DELAY);

		//Überprüfen, ob die Simulation erfolgreich gestartet wurde
		/*if (myBuffer == 30){
			printf("Simulation erfolgreich gestartet");
		}
		else printf("Fehler beim Starten der Simulation");
		vTaskDelay(1000);*/
	}
			

}

void loadingStationFunc(void* pvParameters){
	int status = -1;
	while (true)
	{
		status = *(int*)pvParameters;
		if (status == START_AREA_ENTRY){
			puts("Fahrzeug auf Startposition. \n");
			taskYIELD();
		}
		else puts("Kein Fahrzeug erkannt. \n");
		vTaskDelay(100);
	}
}
