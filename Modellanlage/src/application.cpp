/**************************************************************************************************
Module:		application.cpp
Purpose:	This template provides a FreeRTOS Windows Simulation
System:		Visual Studio / Windows - FreeRTOS Simulator
Date:		2016-03-11
Version:	2.1.0
**************************************************************************************************/
#include <FreeRTOS.h>
#include <stdio.h>
#include <iostream>
#include "task.h"
#include "util\bprint.h"
#include "util\simCommunication.h"
#include "ComCodes.h"

using namespace std;

//Funktionsprototypen
void startAreaFunc(void* pvParameters);
void loadingStationFunc(void* pvParameters);

//Queues
xQueueHandle statusQueue;
xQueueHandle startQueue;
xQueueHandle loadQueue;

//Semaphores
SemaphoreHandle_t loadingStationCount;
SemaphoreHandle_t loadingStationAccess;

//TaskHandle
TaskHandle_t startArea_t;
TaskHandle_t loadStation_t;



/******* Insert your code in this task **********************************************************/
extern "C" void taskApplication(void *pvParameters)
{
	(void*)pvParameters; // Prevent unused warning

	int myBuffer;
	void* pvMyBuffer = &myBuffer;
	int startbuffer;
	int loadbuffer;

	//Tasknamen
	const char scale[] = "scale";
	const char waitStation[] = "waitStation";
	const char startArea[] = "startArea";
	const char loadStation[] = "loadStation";
	const char unloadStation[] = "unloadStation";

	vTaskPrioritySet(NULL, tskIDLE_PRIORITY + 6);

	//Initialisieren des Simulators
	//initSystem() gibt die Message Queue zurück, auf der die Nachrichten des Simulators empfangen werden.
	statusQueue = initSystem();
	vTaskDelay(5000);

	//Initialisieren der Semaphoren
	loadingStationCount = xSemaphoreCreateCounting(2, 2);
	loadingStationAccess = xSemaphoreCreateBinary();
	xSemaphoreGive(loadingStationAccess);
	

	xQueueReceive(statusQueue, &myBuffer, portMAX_DELAY);
	if (myBuffer == 30) {
		printf("Simulation erfolgreich gestartet. \n");
		xTaskCreate(startAreaFunc, startArea, configMINIMAL_STACK_SIZE, pvMyBuffer, tskIDLE_PRIORITY + 1, &startArea_t);
		xTaskCreate(loadingStationFunc, loadStation, configMINIMAL_STACK_SIZE, pvMyBuffer, tskIDLE_PRIORITY + 2, &loadStation_t);

		//Erzeugen der einzelnen Message Queues
		startQueue = xQueueCreate(10, sizeof(int));
		loadQueue = xQueueCreate(10, sizeof(int));
	}
	else puts("Simulation konnte nicht gestartet werden. \n");
	
	//vTaskPrioritySet(NULL, tskIDLE_PRIORITY + 4);
	// Start des Steuerprogramms
	while (true)
	{
		if (xQueueReceive(statusQueue, &myBuffer, portMAX_DELAY)){
			printf("Daten %i empfangen. \n", myBuffer);
			startbuffer = myBuffer;
			loadbuffer = myBuffer;
			xQueueSend(startQueue, &startbuffer, portMAX_DELAY);
			xQueueSend(loadQueue, &loadbuffer, portMAX_DELAY);
		}
		else puts("keine Daten.\n");
	}
			

}

void startAreaFunc(void* pvParameters){
	BaseType_t status = -1;
	int error = -1;
	int buffer;
	
	//Zwei Semaphoren, eine für die Zufahrt zur Beladestation und eine für die freien Plätze auf der Beladestation
	BaseType_t roadwayFree, loadPlaceFree;

	while (true)
	{
		//status = *(int*)pvParameters;
		status = xQueueReceive(startQueue, &buffer, portMAX_DELAY);
		printf("Startbereich gestartet.\n");

		//Wenn ein Fahrzeug im Startbereich steht, kann es versuchen die Semaphore zu bekommen
		//Überprüfen, ob die Semaphore frei ist.
		if (buffer == START_AREA_ENTRY){
			if (xSemaphoreTake(loadingStationAccess, 20) == pdFALSE) {
				sendTo(START_AREA_STOP, STOP_ACTIVE);
				printf("Weg nicht frei.\n");
			}
			else{
				//Überprüfen, ob ein Beladeplatz frei ist
				//Falls, ja darf das Fahrzeug starten
				sendTo(START_AREA_STOP, STOP_ACTIVE);
				printf("Fahrzeug im Startbereich\n");
				if (xSemaphoreTake(loadingStationCount, 100) == pdTRUE) sendTo(START_AREA_STOP, STOP_INACTIVE);
				else {
					sendTo(START_AREA_STOP, STOP_ACTIVE);
					printf("Alle Stationen belegt.\n");
				}
			}
		}
		else if (buffer == START_AREA_EXIT)
		{
			sendTo(START_AREA_STOP, STOP_ACTIVE);
		}
		else vTaskDelay(5); /*allow task switch*/
		//vTaskDelay(5);
		
	}

}


void loadingStationFunc(void* pvParameters){
	//Variablen
	BaseType_t status = -1;
	int buffer;
	bool loadPlace1 = true;
	bool loadPlace2 = true;

	//Task-Funktion
	while (true)
	{
		//status = *(int*)pvParameters;
		status = xQueueReceive(loadQueue, &buffer, portMAX_DELAY);
		printf("Beladestation aktiv.\n");
		//Beladestation 1
		if (buffer == LOAD_PLACE_1_ENTRY && loadPlace1){
			loadPlace1 = false;
			xSemaphoreGive(loadingStationAccess);
			sendTo(SWITCH_LOAD_PLACE, LOAD_PLACE_2);
			printf("Weiche nach 2 gestellt.\n");
			sendTo(LOADING_1_START, LOADING_1_START);
			printf("Beladen 1 gestartet.\n");
		}
		//Beladen 1 beendet?
		else if (buffer == LOADING_1_END ){
			sendTo(LOAD_PLACE_1_STOP, STOP_INACTIVE);
		}
		// Beladestation 1 verlassen?
		else if (buffer == LOAD_PLACE_1_EXIT && !loadPlace1){
			sendTo(LOAD_PLACE_1_STOP, STOP_ACTIVE);
			sendTo(SWITCH_LOAD_PLACE, LOAD_PLACE_1);
			loadPlace1 = true;
			xSemaphoreGive(loadingStationCount);
			printf("Zählsemaphore zurückgeben.\n");
		}
				
		//Beladestation 2
		else if (buffer == LOAD_PLACE_2_ENTRY && loadPlace2){
			loadPlace2 = false;
			xSemaphoreGive(loadingStationAccess);
			sendTo(SWITCH_LOAD_PLACE, LOAD_PLACE_1);
			printf("Weiche nach 1 gestellt.\n");
			sendTo(LOADING_2_START, LOADING_2_START);
			printf("Beladen 2 gestartet.\n");
		}
		//Beladen 2 beendet?
		else if (buffer == LOADING_2_END){
			sendTo(LOAD_PLACE_2_STOP, STOP_INACTIVE);
		}
		// Beladestation 2 verlassen?
		else if (buffer == LOAD_PLACE_2_EXIT && !loadPlace2){
			sendTo(LOAD_PLACE_2_STOP, STOP_ACTIVE);
			sendTo(SWITCH_LOAD_PLACE, LOAD_PLACE_2);
			loadPlace2 = true;
			xSemaphoreGive(loadingStationCount);
			printf("Zählsemaphore zurückgeben.\n");
		}
		//Warten
		else vTaskDelay(5); /*allow task switch*/
		
		//Taskwechsel erzwingen
		//taskYIELD();
	}
}