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
#include "ThreePlaceWaitStation.h"

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
SemaphoreHandle_t unloadSwitch2;
SemaphoreHandle_t unloadPlace3Access;

//TaskHandle
TaskHandle_t startArea_t;
TaskHandle_t loadStation_t;
TaskHandle_t scale_t;
TaskHandle_t unloadStation_t;
TaskHandle_t waitStation_t;

//Stationen der Modellanlage
StartArea Start1;
LoadingStation Load1;
ThreePlaceWaitStation UnloadingStation;

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

	//Erzeugen der Message Queues
	startQueue = xQueueCreate(20, sizeof(int));
	loadQueue = xQueueCreate(20, sizeof(int));
	scaleQueue = xQueueCreate(15, sizeof(int));
	unloadQueue = xQueueCreate(20, sizeof(int));
	waitQueue = xQueueCreate(15, sizeof(int));

	//Initialisieren des Simulators
	//initSystem() gibt die Message Queue zurück, auf der die Nachrichten des Simulators empfangen werden.
	statusQueue = initSystem();
	if (UnloadingStation.stationInit() == 0){
		printf("Beladestation erfolgreich initialisiert.\n");
	}
	else puts("Beladestation konnte nicht initialisiert werden.\n");

	vTaskDelay(4000); /*Wartezeit für die Initialisierung des Systems*/

	//Initialisieren der Semaphoren
	loadingStationCount = xSemaphoreCreateCounting(2, 2);
	loadingStationAccess = xSemaphoreCreateBinary();
	scaleAccess = xSemaphoreCreateBinary();
	xSemaphoreGive(loadingStationAccess);
	unloadingStationCount = xSemaphoreCreateCounting(3, 3);
	unloadingStationAccess = xSemaphoreCreateBinary();
	waitStationAccess = xSemaphoreCreateBinary();
	unloadSwitch2 = xSemaphoreCreateBinary();

	//Initialsieren der Stationen
	//Load1.stationInit();
	Load1.setRoadwayStat(true);


	xQueueReceive(statusQueue, &myBuffer, portMAX_DELAY);
	if (myBuffer == 30) {
		printf("Simulation erfolgreich gestartet. \n");
		xTaskCreate(startAreaFunc, startArea, configMINIMAL_STACK_SIZE, pvMyBuffer, tskIDLE_PRIORITY + 2, &startArea_t);
		xTaskCreate(loadingStationFunc, loadStation, configMINIMAL_STACK_SIZE, pvMyBuffer, tskIDLE_PRIORITY + 1, &loadStation_t);
		xTaskCreate(scaleFunc, scale, configMINIMAL_STACK_SIZE, pvMyBuffer, tskIDLE_PRIORITY + 5, &scale_t);
		xTaskCreate(unloadingStationFunc, unloadStation, configMINIMAL_STACK_SIZE, pvMyBuffer, tskIDLE_PRIORITY + 3, &unloadStation_t);
		xTaskCreate(waitStationFunc, waitStation, configMINIMAL_STACK_SIZE, pvMyBuffer, tskIDLE_PRIORITY + 4, &waitStation_t);
	}
	else puts("Simulation konnte nicht erfolgreich gestartet werden. \n");

	//vTaskPrioritySet(NULL, tskIDLE_PRIORITY + 4);
	// Start des Steuerprogramms
	while (true)
	{
		if (xQueueReceive(statusQueue, &myBuffer, portMAX_DELAY)){
			//printf("Daten %i empfangen. \n", myBuffer);

			/*Die Ereignisse der Simulation werden in den Kommunikationsbuffer,
			der einzelnen Tasks kopiert*/
			startQueueBuffer = myBuffer;
			loadQueueBuffer = myBuffer;
			scaleQueueBuffer = myBuffer;
			unloadQueueBuffer = myBuffer;
			waitQueueBuffer = myBuffer;

			/*Senden der einzelnen Ereignisse an die jeweiligen Tasks.
			Die TicksToWait wurden auf 10 eingestellt, damit die Empfangstask nicht
			zulange blockiert, wenn eine Message Queue keine neuen Ereignisse entgegen nehmen kann.*/
			xQueueSend(startQueue, &startQueueBuffer, 10);
			xQueueSend(loadQueue, &loadQueueBuffer, 10);
			xQueueSend(scaleQueue, &scaleQueueBuffer, 10);
			xQueueSend(unloadQueue, &unloadQueueBuffer, 10);
			xQueueSend(waitQueue, &waitQueueBuffer, 10);

			/*Alle Ereignisse bei denen ein Fahrzeug die Station verlässt,
			werden in der Application Task ausgewertet, damit die Reaktion immer rechtzeitig erfolgt.*/
			switch (myBuffer)
			{
			case START_AREA_EXIT:
				sendTo(START_AREA_STOP, STOP_ACTIVE);
				break;

			case LOAD_PLACE_1_EXIT:
				sendTo(LOAD_PLACE_1_STOP, STOP_ACTIVE);
				sendTo(SWITCH_LOAD_PLACE, LOAD_PLACE_1);
				xSemaphoreGive(loadingStationCount);
				break;

			case LOAD_PLACE_2_EXIT:
				sendTo(LOAD_PLACE_2_STOP, STOP_ACTIVE);
				sendTo(SWITCH_LOAD_PLACE, LOAD_PLACE_2);
				xSemaphoreGive(loadingStationCount);
				break;

			case UNLOAD_PLACE_1_EXIT:
				xSemaphoreGive(unloadingStationCount);
				if (UnloadingStation.stationRelease(0) == -1) puts("Ein Fehler beim Freigeben ist aufgetreten.\n");
				sendTo(UNLOAD_PLACE_1_STOP, STOP_ACTIVE);
				break;

			case UNLOAD_PLACE_2_EXIT:
				xSemaphoreGive(unloadingStationCount);
				if (UnloadingStation.stationRelease(1) == -1) puts("Ein Fehler beim Freigeben ist aufgetreten.\n");
				sendTo(UNLOAD_PLACE_2_STOP, STOP_ACTIVE);
				break;

			case UNLOAD_PLACE_3_EXIT:
				xSemaphoreGive(unloadingStationCount);
				if (UnloadingStation.stationRelease(2) == -1) puts("Ein Fehler beim Freigeben ist aufgetreten.\n");
				sendTo(UNLOAD_PLACE_3_STOP, STOP_ACTIVE);
				break;

			case SCALE_START_EXIT:
				sendTo(SCALE_START_STOP, STOP_ACTIVE);
				xSemaphoreGive(scaleAccess);
				break;

			case SCALE_UNLOAD_EXIT:
				sendTo(SCALE_UNLOAD_STOP, STOP_ACTIVE);
				xSemaphoreGive(scaleAccess);
				break;

			case WAITSTATION_EXIT:
				sendTo(WAITSTATION_STOP, STOP_ACTIVE);
				xSemaphoreGive(waitStationAccess);
				break;

			case ENDSEQUENZ:
				sendTo(START_AREA_STOP, STOP_ACTIVE);
				vTaskSuspend(startArea_t);
				break;

			default:
				vTaskDelay(1);
				break;
			}
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
	int16_t buffer;

	while (true)
	{
		//status = *(int*)pvParameters;

		//printf("Startbereich gestartet.\n");
		xQueueReceive(startQueue, &buffer, portMAX_DELAY); /*Task wartet bei jedem Schleifendurchlauf auf den Empfang eines neuen Ereignisses*/
		//printf("Startbereich %i empfangen.\n", buffer);

		switch (buffer){
		case START_AREA_ENTRY:
			//Überprüfen, ob die Strecke frei ist.
			statusRoadway = xSemaphoreTake(loadingStationAccess, portMAX_DELAY);
			if (statusRoadway == pdFALSE) {
				printf("Strecke belegt.\n");
			}
			else{
				//statusLoadPlace = Load1.loadPlaceReserve();
				if (xSemaphoreTake(loadingStationCount, portMAX_DELAY) == pdTRUE){
					//printf("Fahrzeug gestartet.\n");
					sendTo(START_AREA_STOP, STOP_INACTIVE);
				}
				else printf("Keine Station frei.\n");
			}
			break;

			//case START_AREA_EXIT:
			//	sendTo(START_AREA_STOP, STOP_ACTIVE);
			//	break;

		default:
			//printf("Startbereich idle.\n");
			vTaskDelay(5); /*allow Task switch*/
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

	/*Initialisierung der Semaphoren*/
	xSemaphoreGive(scaleAccess);

	/*Initialisierung der Weiche*/
	sendTo(SWITCH_LOAD_PLACE, LOAD_PLACE_1);

	//Task-Funktion
	while (true)
	{
		//status = *(int*)pvParameters;
		//printf("Beladestation aktiv.\n");
		xQueueReceive(loadQueue, &buffer, portMAX_DELAY); /*Task wartet bei jedem Schleifendurchlauf auf den Empfang eines neuen Ereignisses*/
		//printf("Beladestation %i empfangen.\n", buffer);

		//Beladestation 1
		if (buffer == LOAD_PLACE_1_ENTRY){
			sendTo(SWITCH_LOAD_PLACE, LOAD_PLACE_2);
			sendTo(LOADING_1_START, LOADING_1_START);
			xSemaphoreGive(loadingStationAccess);
		}
		//Beladen 1 beendet?
		else if (buffer == LOADING_1_END){
			//Prüfen, ob die Waage befahren werden kann?
			if (xSemaphoreTake(unloadingStationCount, portMAX_DELAY) == pdTRUE) {
				if (xSemaphoreTake(scaleAccess, portMAX_DELAY) == pdTRUE) sendTo(LOAD_PLACE_1_STOP, STOP_INACTIVE);
			}

		}
		// Beladestation 1 verlassen?
		//else if (buffer == LOAD_PLACE_1_EXIT){
		//	sendTo(LOAD_PLACE_1_STOP, STOP_ACTIVE);
		//	sendTo(SWITCH_LOAD_PLACE, LOAD_PLACE_1);
		//	xSemaphoreGive(loadingStationCount);
		//}

		//Beladestation 2
		else if (buffer == LOAD_PLACE_2_ENTRY){
			sendTo(SWITCH_LOAD_PLACE, LOAD_PLACE_1);
			sendTo(LOADING_2_START, LOADING_2_START);
			xSemaphoreGive(loadingStationAccess);
		}
		//Beladen 2 beendet?
		else if (buffer == LOADING_2_END){
			if (xSemaphoreTake(unloadingStationCount, portMAX_DELAY) == pdTRUE) {
				if (xSemaphoreTake(scaleAccess, portMAX_DELAY) == pdTRUE)  sendTo(LOAD_PLACE_2_STOP, STOP_INACTIVE);
				//vTaskDelay(10);
			}
		}
		// Beladestation 2 verlassen?
		//else if (buffer == LOAD_PLACE_2_EXIT){
		//	sendTo(LOAD_PLACE_2_STOP, STOP_ACTIVE);
		//	sendTo(SWITCH_LOAD_PLACE, LOAD_PLACE_2);
		//	xSemaphoreGive(loadingStationCount);
		//}

		//Warten
		else {
			//printf("Beladestation idle.\n");
			vTaskDelay(5);	/*allow Task switch*/
		}
	}
}

void scaleFunc(void* pvParameters){
	//Task für die Funktion der Waage
	int status;	// Variable für die Message Queue
	bool scaleAvail = true;
	bool directionUnload = true;
	int buffer;
	int16_t rc;

	//Endlosschleife für die Taskfunktion
	while (true)
	{
		//status = *(int*)pvParameters;

		printf("Waage aktiv.\n");
		xQueueReceive(scaleQueue, &buffer, portMAX_DELAY); /*Task wartet bei jedem Schleifendurchlauf auf den Empfang eines neuen Ereignisses*/
		printf("Waage %i empfangen.\n", buffer);

		//Fahrzeug auf Waage eingefahren Richtung Startbereich?
		if (buffer == SCALE_START_ENTRY){
			directionUnload = false;
			printf("Waage ausfahrt Richtung Start.\n");
			sendTo(SCALE_START, SCALE_START);			//Wiegevorgang starten
			//directionStart = true;
		}

		//Fahrzeug hat Waage verlassen Richtung Startbereich?
		//else if (buffer == SCALE_START_EXIT){
		//	sendTo(SCALE_START_STOP, STOP_ACTIVE);
		//	xSemaphoreGive(scaleAccess);
		//}

		//Wiegevorgang beendet?
		else if (buffer == SCALE_END){
			if (directionUnload == false){
				sendTo(SCALE_START_STOP, STOP_INACTIVE);
				vTaskDelay(5);
			}
			else{
				rc = UnloadingStation.stationReserve();
				if (rc == 0){
					sendTo(SWITCH_UNLOAD_1, UNLOAD_PLACE_1);
					sendTo(SCALE_START_STOP, STOP_INACTIVE);
				}
				else if (rc == 1){
					sendTo(SWITCH_UNLOAD_1, UNLOAD_PLACE_2);
					sendTo(SCALE_START_STOP, STOP_INACTIVE);
				}
				else if (rc == 2){
					printf("Waage wartet auf Weichen-Semaphore.\n");
					if (xSemaphoreTake(unloadSwitch2, portMAX_DELAY) == pdTRUE){
						printf("Waage hat die Weichen-Semaphore bekommen.\n");
						sendTo(SWITCH_UNLOAD_2, UNLOAD_PLACE_3);
						sendTo(SWITCH_UNLOAD_1, UNLOAD_STRAIGHT_1);
						sendTo(SCALE_START_STOP, STOP_INACTIVE);
					}
				}
				else puts("Ein Fehler ist aufgetreten.\n");
			}
		}

		else if (buffer == UNLOAD_PLACE_3_ENTRY){
			xSemaphoreGive(unloadSwitch2);
			printf("Weichen-Semaphore frei.\n");
		}

		//Fahrzeug auf Waage eingefahren Richtung Entladenbereich?
		else if (buffer == SCALE_UNLOAD_ENTRY){
			directionUnload = true;
			sendTo(SCALE_START, SCALE_START);			//Wiegevorgang starten
		}

		//Fahrzeug hat Waage verlassen Richtung Entladenbereich?
		//else if (buffer == SCALE_UNLOAD_EXIT)
		//{
		//	sendTo(SCALE_UNLOAD_STOP, STOP_ACTIVE);
		//	xSemaphoreGive(scaleAccess);
		//}

		else vTaskDelay(4); /*allow Task switch*/
	}
}

void unloadingStationFunc(void* pvParameters){
	/*Variablen*/
	int buffer;

	/*Initialisierung der Semaphoren*/
	xSemaphoreGive(unloadingStationAccess);
	xSemaphoreGive(unloadSwitch2);

	/*Initialisierung der Weichen*/
	sendTo(SWITCH_UNLOAD_1, UNLOAD_PLACE_1);
	sendTo(SWITCH_UNLOAD_2, UNLOAD_STRAIGHT_2);

	/*Endlosschleife für die Funktion*/
	while (true)
	{
		//printf("Entladestation aktiv.\n");
		xQueueReceive(unloadQueue, &buffer, portMAX_DELAY);
		//printf("Entladestation %i empfangen.\n", buffer);

		switch (buffer)
		{
		case UNLOAD_PLACE_1_ENTRY:
			xSemaphoreGive(unloadingStationAccess);
			//sendTo(SWITCH_UNLOAD_1, UNLOAD_PLACE_2);		/*Entladestation auf Platz 2 einstellen*/
			sendTo(UNLOADING_1_START, UNLOADING_1_START);	/*Fahrzeug entladen*/
			break;

		case UNLOAD_PLACE_2_ENTRY:
			xSemaphoreGive(unloadingStationAccess);
			//sendTo(SWITCH_UNLOAD_1, UNLOAD_STRAIGHT_1);		/*Entladestation auf Platz 3 einstellen*/
			//sendTo(SWITCH_UNLOAD_2, UNLOAD_PLACE_3);
			sendTo(UNLOADING_2_START, UNLOADING_2_START);	/*Fahrzeug entladen*/
			break;

		case UNLOAD_PLACE_3_ENTRY:
			//xSemaphoreGive(unloadSwitch2);
			xSemaphoreGive(unloadingStationAccess);
			//sendTo(SWITCH_UNLOAD_1, UNLOAD_PLACE_1);		/*Entladestation auf Platz 1 einstellen*/
			//sendTo(SWITCH_UNLOAD_2, UNLOAD_STRAIGHT_2);
			sendTo(UNLOADING_3_START, UNLOADING_3_START);	/*Fahrzeug entladen*/
			break;

		case UNLOADING_1_END:
			if (xSemaphoreTake(waitStationAccess, portMAX_DELAY) == pdTRUE){
				printf("Enladeplatz 1 wartet auf Semaphore.\n");
				if (xSemaphoreTake(unloadSwitch2, portMAX_DELAY) == pdTRUE){
					printf("Enladeplatz 1 hat die Semaphore bekommen.\n");
					sendTo(SWITCH_UNLOAD_2, UNLOAD_STRAIGHT_2);
					sendTo(UNLOAD_PLACE_1_STOP, STOP_INACTIVE);
				}
			}
			else printf("Unable to aquire semaphore waitStationAccess.\n");
			break;

		case UNLOADING_2_END:
			if (xSemaphoreTake(waitStationAccess, portMAX_DELAY) == pdTRUE){
				printf("Enladeplatz 2 wartet auf Semaphore.\n");
				if (xSemaphoreTake(unloadSwitch2, portMAX_DELAY) == pdTRUE){
					printf("Enladeplatz 2 hat die Semaphore bekommen.\n");
					sendTo(SWITCH_UNLOAD_2, UNLOAD_STRAIGHT_2);
					sendTo(UNLOAD_PLACE_2_STOP, STOP_INACTIVE);
				}
			}
			else printf("Unable to aquire semaphore waitStationAccess.\n");
			break;


		case UNLOADING_3_END:
			if (xSemaphoreTake(waitStationAccess, portMAX_DELAY) == pdTRUE){
				sendTo(UNLOAD_PLACE_3_STOP, STOP_INACTIVE);
			}
			else printf("Unable to aquire semaphore waitStationAccess.\n");
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
		printf("Wartebereich aktiv.\n");
		xQueueReceive(waitQueue, &buffer, portMAX_DELAY);
		printf("Wartebereich %i empfangen.\n", buffer);

		switch (buffer)
		{
		case WAITSTATION_ENTRY:
			xSemaphoreGive(unloadSwitch2);
			printf("WaitStation: Weichen Semaphore freigegeben.\n");
			if (xSemaphoreTake(scaleAccess, portMAX_DELAY) == pdTRUE){
				sendTo(WAITSTATION_STOP, STOP_INACTIVE);
			}
			else puts("Waage konnte nicht belegt werden.\n");
			break;

			//case WAITSTATION_EXIT:
			//	sendTo(WAITSTATION_STOP, STOP_ACTIVE);
			//	xSemaphoreGive(waitStationAccess);
			//	break;

		default:
			vTaskDelay(5);
			break;
		}


	}
}