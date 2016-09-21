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
#include "StartArea.h"
#include "LoadingStation.h"

using namespace std;

//Funktionsprototypen
void startAreaFunc(void* pvParameters);			/*Taskfunktion zur Steuerung des Startbereichs*/
void loadingStationFunc(void* pvParameters);	/*Taskfunktion zur Steuerung der Beladestation*/
void scaleFunc(void* pvParameters);				/*Taskfunktion zur Steuerung der Waage*/
void unloadingStationFunc(void* pvParameters);	/*Taskfunktion zur Steuerung der Entladestation*/
void waitStationFunc(void* pvParameters);		/*Taskfunktion zur Steuerung des Wartebereichs*/

//Queues
xQueueHandle statusQueue;	/*Message Queue für die Kommunikation mit dem Simulator*/
xQueueHandle startQueue;	/*Message Queue für den Startbereich*/
xQueueHandle loadQueue;		/*Message Queue für die Beladestation*/
xQueueHandle scaleQueue;	/*Message Queue für die Waage*/
xQueueHandle unloadQueue;	/*Message Queue für die Entladestation*/
xQueueHandle waitQueue;		/*Message Queue für den Wartebereich*/

//Semaphores
SemaphoreHandle_t loadingStationCount;
SemaphoreHandle_t loadingStationAccess;
SemaphoreHandle_t scaleAccess;
SemaphoreHandle_t unloadingStationCount;
SemaphoreHandle_t unloadingStationAccess;
SemaphoreHandle_t waitStationAccess;

//TaskHandle
TaskHandle_t startArea_t;
TaskHandle_t loadStation_t;
TaskHandle_t scale_t;
TaskHandle_t unloadStation_t;
TaskHandle_t waitStation_t;

//Stationen der Modellanlage
StartArea Start1;
LoadingStation Load1;

/******* Insert your code in this task **********************************************************/
extern "C" void taskApplication(void *pvParameters)
{
	(void*)pvParameters; // Prevent unused warning

	//Variablen
	int myBuffer;
	void* pvMyBuffer = &myBuffer;	//Nachrichtenbuffer mit Typ-Cast
	int startQueueBuffer;
	int loadQueueBuffer;
	int scaleQueueBuffer;
	int unloadQueueBuffer;
	int waitQueueBuffer;

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
	vTaskDelay(5000); /*Wartezeit für die Initialisierung des Systems*/

	//Initialisieren der Semaphoren
	loadingStationCount = xSemaphoreCreateCounting(2, 2);
	loadingStationAccess = xSemaphoreCreateBinary();
	scaleAccess = xSemaphoreCreateBinary();
	xSemaphoreGive(loadingStationAccess);
	unloadingStationCount = xSemaphoreCreateCounting(3, 3);
	unloadingStationAccess = xSemaphoreCreateBinary();
	waitStationAccess = xSemaphoreCreateBinary();

	//Initialsieren der Stationen
	//Load1.stationInit();
	Load1.setRoadwayStat(true);


	xQueueReceive(statusQueue, &myBuffer, portMAX_DELAY);
	if (myBuffer == 30) {
		printf("Simulation erfolgreich gestartet. \n");
		xTaskCreate(startAreaFunc, startArea, configMINIMAL_STACK_SIZE, pvMyBuffer, tskIDLE_PRIORITY + 2, &startArea_t);
		xTaskCreate(loadingStationFunc, loadStation, configMINIMAL_STACK_SIZE, pvMyBuffer, tskIDLE_PRIORITY + 1, &loadStation_t);
		xTaskCreate(scaleFunc, scale, configMINIMAL_STACK_SIZE, pvMyBuffer, tskIDLE_PRIORITY + 4, &scale_t);
		xTaskCreate(unloadingStationFunc, unloadStation, configMINIMAL_STACK_SIZE, pvMyBuffer, tskIDLE_PRIORITY + 1, &unloadStation_t);
		xTaskCreate(waitStationFunc, waitStation, configMINIMAL_STACK_SIZE, pvMyBuffer, tskIDLE_PRIORITY + 3, &waitStation_t);


		//Erzeugen der Message Queues
		startQueue = xQueueCreate(30, sizeof(int));
		loadQueue = xQueueCreate(30, sizeof(int));
		scaleQueue = xQueueCreate(30, sizeof(int));
		unloadQueue = xQueueCreate(30, sizeof(int));
		waitQueue = xQueueCreate(30, sizeof(int));
	}
	else puts("Simulation konnte nicht erfolgreich gestartet werden. \n");

	//vTaskPrioritySet(NULL, tskIDLE_PRIORITY + 4);
	// Start des Steuerprogramms
	while (true)
	{
		if (xQueueReceive(statusQueue, &myBuffer, portMAX_DELAY)){
			printf("Daten %i empfangen. \n", myBuffer);
			startQueueBuffer = myBuffer;
			loadQueueBuffer = myBuffer;
			scaleQueueBuffer = myBuffer;
			unloadQueueBuffer = myBuffer;
			waitQueueBuffer = myBuffer;

			xQueueSend(startQueue, &startQueueBuffer, portMAX_DELAY);
			xQueueSend(loadQueue, &loadQueueBuffer, portMAX_DELAY);
			xQueueSend(scaleQueue, &scaleQueueBuffer, portMAX_DELAY);
			xQueueSend(unloadQueue, &unloadQueueBuffer, portMAX_DELAY);
			xQueueSend(waitQueue, &waitQueueBuffer, portMAX_DELAY);
		}
		else puts("keine Daten.\n");
	}
}

void startAreaFunc(void* pvParameters){
	int16_t status = -1;
	int16_t error = -1;
	bool vehicleStarted = false;
	bool statusRoadway = true;
	bool stopActor = true;

	int16_t statusLoadPlace = 0;
	int buffer;

	while (true)
	{
		//status = *(int*)pvParameters;

		printf("Task 1 gestartet.\n");
		xQueueReceive(startQueue, &buffer, portMAX_DELAY); /*Task wartet bei jedem Schleifendurchlauf auf den Empfang eines neuen Ereignisses*/

		switch (buffer){
		case START_AREA_ENTRY:
			//Überprüfen, ob die Strecke frei ist.
			statusRoadway = xSemaphoreTake(loadingStationAccess, portMAX_DELAY);
			if (statusRoadway == pdFALSE) {
				printf("Strecke belegt.\n");
				vTaskDelay(1);
			}
			else{
				//statusLoadPlace = Load1.loadPlaceReserve();
				if (xSemaphoreTake(loadingStationCount,portMAX_DELAY)==pdTRUE){
					//statusRoadway = false;
					//Load1.setRoadwayStat(false);
					printf("Fahrzeug gestartet.\n");
					sendTo(START_AREA_STOP, STOP_INACTIVE);
					//stopActor = false;
					//vTaskDelay(5);		//Verzögerung, damit der LKW ausreichend Zeit zum Starten hat.
				}
				else printf("Keine Station frei.\n");
				//error = Load1.stationReserve();
				//if (error == -1) printf("Es konnte keine Station reserviert werden.\n");
			}
			break;

		case START_AREA_EXIT:
			sendTo(START_AREA_STOP, STOP_ACTIVE);
			break;

		default:
			printf("Task 1 idle.\n");
			vTaskDelay(2); /*allow Task switch*/
			break;
		}
	}
}

void loadingStationFunc(void* pvParameters){
	//Variablen
	int status = -1;
	int buffer;
	bool loadPlace1 = true;
	bool loadPlace2 = true;

	xSemaphoreGive(scaleAccess);
	//Task-Funktion
	while (true)
	{
		//status = *(int*)pvParameters;
		printf("Beladestation aktiv.\n");
		xQueueReceive(loadQueue, &buffer, portMAX_DELAY); /*Task wartet bei jedem Schleifendurchlauf auf den Empfang eines neuen Ereignisses*/

		//Beladestation 1
		if (buffer == LOAD_PLACE_1_ENTRY && loadPlace1){
			loadPlace1 = false;
			Load1.setRoadwayStat(true);
			sendTo(SWITCH_LOAD_PLACE, LOAD_PLACE_2);
			printf("Weiche nach 2 gestellt.\n");
			sendTo(LOADING_1_START, LOADING_1_START);
			printf("Beladen 1 gestartet.\n");
			xSemaphoreGive(loadingStationAccess);
			printf("Start -> Beladestation frei.\n");
			vTaskDelay(100);
		}
		//Beladen 1 beendet?
		else if (buffer == LOADING_1_END){
			printf("Beladen beendet.\n");

			//Prüfen, ob die Waage befahren werden kann?
			if (xSemaphoreTake(scaleAccess, portMAX_DELAY)) {
				sendTo(LOAD_PLACE_1_STOP, STOP_INACTIVE);
				//vTaskDelay(10);
			}

		}
		// Beladestation 1 verlassen?
		else if (buffer == LOAD_PLACE_1_EXIT && !loadPlace1){
			sendTo(LOAD_PLACE_1_STOP, STOP_ACTIVE);
			sendTo(SWITCH_LOAD_PLACE, LOAD_PLACE_1);
			loadPlace1 = true;
			xSemaphoreGive(loadingStationCount);
			printf("Zählsemaphore zurückgeben.\n");
			//xSemaphoreGive(loadingStationAccess);
			printf("Start -> Beladestation frei.\n");
		}

		//Beladestation 2
		else if (buffer == LOAD_PLACE_2_ENTRY && loadPlace2){
			loadPlace2 = false;
			Load1.setRoadwayStat(true);
			sendTo(SWITCH_LOAD_PLACE, LOAD_PLACE_1);
			printf("Weiche nach 1 gestellt.\n");
			sendTo(LOADING_2_START, LOADING_2_START);
			printf("Beladen 2 gestartet.\n");
			xSemaphoreGive(loadingStationAccess);
			printf("Start -> Beladestation frei.\n");
			vTaskDelay(100);
		}
		//Beladen 2 beendet?
		else if (buffer == LOADING_2_END){

			if (xSemaphoreTake(scaleAccess, portMAX_DELAY)) {
				sendTo(LOAD_PLACE_2_STOP, STOP_INACTIVE);
				//vTaskDelay(10);
			}
		}
		// Beladestation 2 verlassen?
		else if (buffer == LOAD_PLACE_2_EXIT && !loadPlace2){
			sendTo(LOAD_PLACE_2_STOP, STOP_ACTIVE);
			sendTo(SWITCH_LOAD_PLACE, LOAD_PLACE_2);
			loadPlace2 = true;
			xSemaphoreGive(loadingStationCount);
			printf("Zählsemaphore zurückgeben.\n");
			//xSemaphoreGive(loadingStationAccess);
			printf("Start -> Beladestation frei.\n");

		}
		//Warten
		else {
			printf("Beladestation idle.\n");
			vTaskDelay(2);	/*allow Task switch*/
		}
	}
}

void scaleFunc(void* pvParameters){
	//Task für die Funktion der Waage
	int status;	// Variable für die Message Queue
	bool scaleAvail = true;
	int buffer;

	//Endlosschleife für die Taskfunktion
	while (true)
	{
		//status = *(int*)pvParameters;

		printf("Waage aktiv.\n");
		xQueueReceive(scaleQueue, &buffer, portMAX_DELAY); /*Task wartet bei jedem Schleifendurchlauf auf den Empfang eines neuen Ereignisses*/

		//Fahrzeug auf Waage eingefahren Richtung Startbereich?
		if (buffer == SCALE_START_ENTRY && scaleAvail){
			sendTo(SCALE_START, SCALE_START);			//Wiegevorgang starten
			scaleAvail = false;
		}

		//Fahrzeug hat Waage verlassen Richtung Startbereich?
		else if (buffer == SCALE_START_EXIT && !scaleAvail){
			sendTo(SCALE_START_STOP, STOP_ACTIVE);
			xSemaphoreGive(scaleAccess);
			scaleAvail = true;
		}

		//Wiegevorgang beendet?
		else if (buffer == SCALE_END){
			sendTo(SCALE_START_STOP, STOP_INACTIVE);
		}

		//Fahrzeug auf Waage eingefahren Richtung Entladenbereich?
		else if (buffer == SCALE_UNLOAD_ENTRY && scaleAvail){
			sendTo(SCALE_START, SCALE_START);			//Wiegevorgang starten
			scaleAvail = false;
		}

		//Fahrzeug hat Waage verlassen Richtung Entladenbereich?
		else if (buffer == SCALE_UNLOAD_EXIT && !scaleAvail){
			sendTo(SCALE_UNLOAD_STOP, STOP_ACTIVE);
			xSemaphoreGive(scaleAccess);
			scaleAvail = true;
		}

		else vTaskDelay(2); /*allow Task switch*/
	}
}

void unloadingStationFunc(void* pvParameters){
	/*Variablen*/
	int buffer;

	/*Initialisierung der Semaphoren*/
	xSemaphoreGive(unloadingStationAccess);

	/*Initialisierung der Weichen*/
	sendTo(SWITCH_UNLOAD_1, UNLOAD_PLACE_1);
	sendTo(SWITCH_UNLOAD_2, UNLOAD_STRAIGHT_2);

	/*Endlosschleife für die Funktion*/
	while (true)
	{
		xQueueReceive(unloadQueue, &buffer, portMAX_DELAY);

		switch (buffer)
		{
		case UNLOAD_PLACE_1_ENTRY:
			xSemaphoreGive(unloadingStationAccess);
			sendTo(SWITCH_UNLOAD_1, UNLOAD_PLACE_2);		/*Entladestation auf Platz 2 einstellen*/
			sendTo(UNLOADING_1_START, UNLOADING_1_START);	/*Fahrzeug entladen*/
			break;

		case UNLOAD_PLACE_2_ENTRY:
			xSemaphoreGive(unloadingStationAccess);
			sendTo(SWITCH_UNLOAD_1, UNLOAD_STRAIGHT_1);		/*Entladestation auf Platz 3 einstellen*/
			sendTo(SWITCH_UNLOAD_2, UNLOAD_PLACE_3);
			sendTo(UNLOADING_2_START, UNLOADING_2_START);	/*Fahrzeug entladen*/
			break;

		case UNLOAD_PLACE_3_ENTRY:
			xSemaphoreGive(unloadingStationAccess);
			sendTo(SWITCH_UNLOAD_1, UNLOAD_PLACE_1);		/*Entladestation auf Platz 1 einstellen*/
			sendTo(SWITCH_UNLOAD_2, UNLOAD_STRAIGHT_2);
			sendTo(UNLOADING_2_START, UNLOADING_2_START);	/*Fahrzeug entladen*/
			break;

		case UNLOADING_1_END:
			if (xSemaphoreTake(waitStationAccess, portMAX_DELAY) == pdTRUE){
				sendTo(SWITCH_LOAD_PLACE, UNLOAD_STRAIGHT_2);
				sendTo(UNLOAD_PLACE_1_STOP, STOP_INACTIVE);
			}
			else printf("Unable to aquire semaphore waitStationAccess.\n");
			break;

		case UNLOAD_PLACE_1_EXIT:
			xSemaphoreGive(unloadingStationCount);
			sendTo(UNLOAD_PLACE_1_STOP, STOP_ACTIVE);
			break;

		case UNLOADING_2_END:
			if (xSemaphoreTake(waitStationAccess, portMAX_DELAY) == pdTRUE){
				sendTo(SWITCH_LOAD_PLACE, UNLOAD_STRAIGHT_2);
				sendTo(UNLOAD_PLACE_2_STOP, STOP_INACTIVE);
			}
			else printf("Unable to aquire semaphore waitStationAccess.\n");
			break;

		case UNLOAD_PLACE_2_EXIT:
			xSemaphoreGive(unloadingStationCount);
			sendTo(UNLOAD_PLACE_2_STOP, STOP_ACTIVE);
			break;

		case UNLOADING_3_END:
			if (xSemaphoreTake(waitStationAccess, portMAX_DELAY) == pdTRUE){
				sendTo(UNLOAD_PLACE_3_STOP, STOP_INACTIVE);
			}
			else printf("Unable to aquire semaphore waitStationAccess.\n");
			break;

		case UNLOAD_PLACE_3_EXIT:
			xSemaphoreGive(unloadingStationCount);
			sendTo(UNLOAD_PLACE_3_STOP, STOP_ACTIVE);
			break;

		default:
			vTaskDelay(2);	/*allow task switch*/
			break;
		}
	}
}

void waitStationFunc(void* pvParameters){
	/*Variablen*/
	int buffer;

	/*Initialisierung der Semaphoren*/
	xSemaphoreGive(waitStationAccess);

	/*Endlosschleife für die Funktion*/
	while (true)
	{
		xQueueReceive(waitQueue, &buffer, portMAX_DELAY);
	}
}