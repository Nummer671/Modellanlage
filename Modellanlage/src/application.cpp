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

//test brench

using namespace std;

//Funktionsprototypen
void startAreaFunc(void* pvParameters);
void loadingStationFunc(void* pvParameters);

//Queues
xQueueHandle statusQueue;

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
		xTaskCreate(startAreaFunc, startArea, configMINIMAL_STACK_SIZE, pvMyBuffer, tskIDLE_PRIORITY + 5, &startArea_t);
		xTaskCreate(loadingStationFunc, loadStation, configMINIMAL_STACK_SIZE, pvMyBuffer, tskIDLE_PRIORITY + 4, &loadStation_t);
	}
	else puts("Simulation konnte nicht gestartet werden. \n");
	
	//vTaskPrioritySet(NULL, tskIDLE_PRIORITY + 4);
	// Start des Steuerprogramms
	while (true)
	{

		if (xQueueReceive(statusQueue, &myBuffer, portMAX_DELAY)){
			puts("Daten empfangen. \n");
		}
		else {
			puts("keine Daten.\n");
		}
		//Überprüfen, ob die Simulation erfolgreich gestartet wurde
		/*if (myBuffer == 30){
			printf("Simulation erfolgreich gestartet");
		}
		else printf("Fehler beim Starten der Simulation");
		vTaskDelay(1000);*/
	}
			

}

void startAreaFunc(void* pvParameters){
	int status = -1;
	int error = -1;
	
	//Zwei Semaphoren, eine für die Zufahrt zur Beladestation und eine für die freien Plätze auf der Beladestation
	BaseType_t roadwayFree, loadPlaceFree;

	while (true)
	{
		status = *(int*)pvParameters;

		roadwayFree = xSemaphoreTake(loadingStationAccess, portMAX_DELAY);
		if (roadwayFree == pdTRUE){
			if (status == START_AREA_ENTRY){
				puts("Fahrzeug auf Startposition. \n");
				loadPlaceFree = xSemaphoreTake(loadingStationCount, portMAX_DELAY);
				if (loadPlaceFree == pdTRUE){
					error = sendTo(START_AREA_STOP, STOP_INACTIVE);
					if (error > 0){
						puts("Fahrzeug gestartet. \n");
					}
					else puts("Fehler!\n");
				}
			}
		}
		else {
			error = sendTo(START_AREA_STOP, STOP_ACTIVE);
			vTaskDelay(1);
		}
	}

}


void loadingStationFunc(void* pvParameters){
	//Variablen
	int status = -1;
	bool loadPlace1 = true;
	bool loadPlace2 = true; 
	

	//Task-Funktion
	while (true)
	{
		status = *(int*)pvParameters;

		//Beladestation 1
		if (status == LOAD_PLACE_1_ENTRY){
			loadPlace1 = false;
			xSemaphoreGive(loadingStationAccess);
			sendTo(SWITCH_LOAD_PLACE, LOAD_PLACE_2);
			sendTo(LOADING_1_START, LOADING_1_START);
			//xSemaphoreGive(loadingStationCount);
			//vTaskResume(startArea_t);
			
		}
		else if (status == LOADING_1_END){
			sendTo(LOAD_PLACE_1_STOP, STOP_INACTIVE);
		}
		else if (status == LOAD_PLACE_1_EXIT){
			sendTo(LOAD_PLACE_1_STOP, STOP_ACTIVE);
			sendTo(SWITCH_LOAD_PLACE, LOAD_PLACE_1);
			loadPlace1 = true;
			xSemaphoreGive(loadingStationCount);
		}
				
		//Beladestation 2
		else if (status == LOAD_PLACE_2_ENTRY){
			loadPlace2 = false;
			xSemaphoreGive(loadingStationAccess);
			sendTo(SWITCH_LOAD_PLACE, LOAD_PLACE_1);
			sendTo(LOADING_2_START, LOADING_2_START);
			//xSemaphoreGive(loadingStationCount);
			vTaskResume(startArea_t);
		}
		else if (status == LOADING_2_END){
			sendTo(LOAD_PLACE_2_STOP, STOP_INACTIVE);
		}
		else if (status == LOAD_PLACE_2_EXIT){
			sendTo(LOAD_PLACE_2_STOP, STOP_ACTIVE);
			sendTo(SWITCH_LOAD_PLACE, LOAD_PLACE_2);
			loadPlace2 = true;
			xSemaphoreGive(loadingStationCount);
			//vTaskResume(startArea_t);
		}
		else vTaskDelay(1);
	}
}