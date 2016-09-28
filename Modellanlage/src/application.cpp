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

	/*Variablen*/
	int16_t buffer;
	int16_t startQueueBuffer;
	int16_t loadQueueBuffer;
	int16_t scaleQueueBuffer;
	int16_t unloadQueueBuffer;
	int16_t waitQueueBuffer;

	/*Tasknamen*/
	const char scale[] = "scale";
	const char waitStation[] = "waitStation";
	const char startArea[] = "startArea";
	const char loadStation[] = "loadStation";
	const char unloadStation[] = "unloadStation";

	/*Setzen der Taskpriorität, damit die Application Task
	immer die höchste Priorität besitzt, da diese Task die 
	Ereignisse des Simulators empfangen muss.*/
	vTaskPrioritySet(NULL, tskIDLE_PRIORITY + 6);

	/*Erzeugen der Message Queues für die Steuerungstasks der 
	einzelnen Stationen. Jede Task bekommt die Ereignisse des 
	Simulators übergeben.*/
	startQueue = xQueueCreate(20, sizeof(int));
	loadQueue = xQueueCreate(20, sizeof(int));
	scaleQueue = xQueueCreate(15, sizeof(int));
	unloadQueue = xQueueCreate(20, sizeof(int));
	waitQueue = xQueueCreate(15, sizeof(int));

	//Initialisieren des Simulators
	//initSystem() gibt die Message Queue zurück, auf der die Nachrichten des Simulators empfangen werden.
	statusQueue = initSystem();

	/*Initialsieren der Stationen*/
	if (UnloadingStation.stationInit() == 0){
		printf("Beladestation erfolgreich initialisiert.\n");
	}
	else puts("Beladestation konnte nicht initialisiert werden.\n");

	vTaskDelay(4000); /*Wartezeit für die Initialisierung des Systems*/

	//Initialisieren der Semaphoren
	loadingStationCount = xSemaphoreCreateCounting(2, 2);
	loadingStationAccess = xSemaphoreCreateBinary();
	scaleAccess = xSemaphoreCreateBinary();
	unloadingStationCount = xSemaphoreCreateCounting(3, 3);
	unloadingStationAccess = xSemaphoreCreateBinary();
	waitStationAccess = xSemaphoreCreateBinary();
	unloadSwitch2 = xSemaphoreCreateBinary();

	


	xQueueReceive(statusQueue, &buffer, portMAX_DELAY);
	if (buffer == 30) {
		printf("Simulation erfolgreich gestartet. \n");
		/*Steuerungstasks anlegen*/
		if (xTaskCreate(startAreaFunc, startArea, configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, &startArea_t) == pdFALSE){
			printf("Task für den Startbereich konnte nicht erzeugt werden.\n");
			vTaskDelay(portMAX_DELAY);
		}
		if (xTaskCreate(loadingStationFunc, loadStation, configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &loadStation_t) == pdFALSE){
			printf("Task für die Beladestation konnte nicht erzeugt werden.\n");
			vTaskDelay(portMAX_DELAY);
		}
		if (xTaskCreate(scaleFunc, scale, configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 5, &scale_t) == pdFALSE){
			printf("Task für die Waage konnte nicht erzeugt werden.\n");
			vTaskDelay(portMAX_DELAY);
		}
		if (xTaskCreate(unloadingStationFunc, unloadStation, configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, &unloadStation_t) == pdFALSE){
			printf("Task für die Entladestation konnte nicht erzeugt werden.\n");
			vTaskDelay(portMAX_DELAY);
		}
		if (xTaskCreate(waitStationFunc, waitStation, configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 4, &waitStation_t) == pdFALSE){
			printf("Task für den Wartebereich konnte nicht erzeugt werden.\n");
			vTaskDelay(portMAX_DELAY);
		}
	}
	else puts("Simulation konnte nicht erfolgreich gestartet werden. \n");

	/*Start der Taskfunktion*/
	while (true)
	{
		if (xQueueReceive(statusQueue, &buffer, portMAX_DELAY)){

			/*Die Ereignisse der Simulation werden in den Kommunikationsbuffer,
			der einzelnen Tasks kopiert*/
			startQueueBuffer = buffer;
			loadQueueBuffer = buffer;
			scaleQueueBuffer = buffer;
			unloadQueueBuffer = buffer;
			waitQueueBuffer = buffer;

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
			switch (buffer)
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
	/*Variablen*/
	BaseType_t statusRoadway;
	int16_t buffer;

	(void*)pvParameters; // Prevent unused warning

	while (true)
	{
		/*Task wartet bei jedem Schleifendurchlauf auf den Empfang eines neuen Ereignisses*/
		if (xQueueReceive(startQueue, &buffer, portMAX_DELAY) == pdFALSE){
			printf("Startbereich: Message Queue konnte nicht gelesen werden.\n");
		}
		else{
			switch (buffer){
			case START_AREA_ENTRY:
				/*Prüfen, ob die Strecke zur Beladestation frei ist.*/
				statusRoadway = xSemaphoreTake(loadingStationAccess, portMAX_DELAY);
				if (statusRoadway == pdFALSE) {
					printf("Strecke belegt.\n");
				}
				else{
					if (xSemaphoreTake(loadingStationCount, portMAX_DELAY) == pdTRUE){
						sendTo(START_AREA_STOP, STOP_INACTIVE);
					}
					else printf("Keine Station frei.\n");
				}
				break;

			default:
				vTaskDelay(5); /*allow Task switch*/
				break;
			}
		}
		
	}
}

void loadingStationFunc(void* pvParameters){
	/*Variablen*/
	int16_t buffer;
	(void*)pvParameters; // Prevent unused warning

	/*Initialisierung der Semaphoren*/
	xSemaphoreGive(loadingStationAccess);
	

	/*Initialisierung der Weiche*/
	sendTo(SWITCH_LOAD_PLACE, LOAD_PLACE_1);

	//Task-Funktion
	while (true)
	{
		/*Task wartet bei jedem Schleifendurchlauf auf den Empfang eines neuen Ereignisses*/
		if (xQueueReceive(loadQueue, &buffer, portMAX_DELAY) == pdFALSE){
			printf("Beladestation: Message Queue konnte nicht gelesen werden.\n");
		}
		else{
			switch (buffer)
			{
			case LOAD_PLACE_1_ENTRY:
				sendTo(SWITCH_LOAD_PLACE, LOAD_PLACE_2);
				sendTo(LOADING_1_START, LOADING_1_START);
				xSemaphoreGive(loadingStationAccess);
				break;

			case LOADING_1_END:
				/*Prüfen, ob die Waage befahren werden kann?*/
				if (xSemaphoreTake(unloadingStationCount, portMAX_DELAY) == pdTRUE) {
					if (xSemaphoreTake(scaleAccess, portMAX_DELAY) == pdTRUE) sendTo(LOAD_PLACE_1_STOP, STOP_INACTIVE);
				}
				break;

			case LOAD_PLACE_2_ENTRY:
				sendTo(SWITCH_LOAD_PLACE, LOAD_PLACE_1);
				sendTo(LOADING_2_START, LOADING_2_START);
				xSemaphoreGive(loadingStationAccess);
				break;

			case LOADING_2_END:
				/*Prüfen, ob die Waage befahren werden kann?*/
				if (xSemaphoreTake(unloadingStationCount, portMAX_DELAY) == pdTRUE) {
					if (xSemaphoreTake(scaleAccess, portMAX_DELAY) == pdTRUE)  sendTo(LOAD_PLACE_2_STOP, STOP_INACTIVE);
				}
				break;

			default:
				vTaskDelay(5);	/*allow Task switch*/
				break;
			}
		}
	}
}

void scaleFunc(void* pvParameters){
	/*Variablen*/
	bool directionUnload = true;
	int16_t buffer;
	int16_t rc;

	(void*)pvParameters; // Prevent unused warning

	/*Initialisieren der Semaphoren*/
	xSemaphoreGive(scaleAccess);

	//Endlosschleife für die Taskfunktion
	while (true)
	{
		/*Task wartet bei jedem Schleifendurchlauf auf den Empfang eines neuen Ereignisses*/
		xQueueReceive(scaleQueue, &buffer, portMAX_DELAY); 

		switch (buffer)
		{
		case SCALE_START_ENTRY:
			directionUnload = false;
			sendTo(SCALE_START, SCALE_START);	//Wiegevorgang starten
			break;

		case SCALE_END:
			//Fahrzeug auf Waage Richtung Entladestation eingefahren ?
			if (directionUnload == false){
				sendTo(SCALE_START_STOP, STOP_INACTIVE);
				//vTaskDelay(5);
			}
			else{
				/*Entladestation reservieren*/
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
					if (xSemaphoreTake(unloadSwitch2, portMAX_DELAY) == pdTRUE){
						/*Weichen auf den driten Entladeplatz einstellen*/
						sendTo(SWITCH_UNLOAD_2, UNLOAD_PLACE_3);
						sendTo(SWITCH_UNLOAD_1, UNLOAD_STRAIGHT_1);
						sendTo(SCALE_START_STOP, STOP_INACTIVE);
					}
				}
				else puts("Waage: Ein Fehler beim Reservieren des Entladeplatzes ist aufgetreten.\n");
			}
			break;

		/*Prüfen, ob ein Fahrzeug auf den dritten Entladeplatz gefahren ist,
		um die Semaphore für die Weiche freizugeben.*/
		case UNLOAD_PLACE_3_ENTRY:
			xSemaphoreGive(unloadSwitch2);
			break;

		case SCALE_UNLOAD_ENTRY:
			directionUnload = true;
			sendTo(SCALE_START, SCALE_START);
			break;

		default:
			vTaskDelay(4); /*allow Task switch*/
			break;
		} 
	}
}

void unloadingStationFunc(void* pvParameters){
	/*Variablen*/
	int16_t buffer;

	(void*)pvParameters; // Prevent unused warning

	/*Initialisierung der Semaphoren*/
	xSemaphoreGive(unloadingStationAccess);
	xSemaphoreGive(unloadSwitch2);

	/*Initialisierung der Weichen*/
	sendTo(SWITCH_UNLOAD_1, UNLOAD_PLACE_1);
	sendTo(SWITCH_UNLOAD_2, UNLOAD_STRAIGHT_2);

	/*Endlosschleife für die Funktion*/
	while (true)
	{
		/*Task wartet bei jedem Schleifendurchlauf auf den Empfang eines neuen Ereignisses*/
		xQueueReceive(unloadQueue, &buffer, portMAX_DELAY);

		switch (buffer)
		{
		case UNLOAD_PLACE_1_ENTRY:
			xSemaphoreGive(unloadingStationAccess);
			sendTo(UNLOADING_1_START, UNLOADING_1_START);	/*Fahrzeug entladen*/
			break;

		case UNLOAD_PLACE_2_ENTRY:
			xSemaphoreGive(unloadingStationAccess);

			sendTo(UNLOADING_2_START, UNLOADING_2_START);	/*Fahrzeug entladen*/
			break;

		case UNLOAD_PLACE_3_ENTRY:
			xSemaphoreGive(unloadingStationAccess);
			sendTo(UNLOADING_3_START, UNLOADING_3_START);	/*Fahrzeug entladen*/
			break;

		case UNLOADING_1_END:
			if (xSemaphoreTake(waitStationAccess, portMAX_DELAY) == pdTRUE){
				if (xSemaphoreTake(unloadSwitch2, portMAX_DELAY) == pdTRUE){
					sendTo(SWITCH_UNLOAD_2, UNLOAD_STRAIGHT_2);
					sendTo(UNLOAD_PLACE_1_STOP, STOP_INACTIVE);
				}
			}
			else printf("Unable to aquire semaphore waitStationAccess.\n");
			break;

		case UNLOADING_2_END:
			if (xSemaphoreTake(waitStationAccess, portMAX_DELAY) == pdTRUE){
				if (xSemaphoreTake(unloadSwitch2, portMAX_DELAY) == pdTRUE){
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
	int16_t buffer;
	(void*)pvParameters; // Prevent unused warning

	/*Initialisierung der Semaphoren*/
	xSemaphoreGive(waitStationAccess);

	/*Endlosschleife für die Funktion*/
	while (true)
	{
		/*Task wartet bei jedem Schleifendurchlauf auf den Empfang eines neuen Ereignisses*/
		xQueueReceive(waitQueue, &buffer, portMAX_DELAY);
	
		switch (buffer)
		{
		case WAITSTATION_ENTRY:
			xSemaphoreGive(unloadSwitch2);		//Zugriff auf die zweite Weiche der Entladestation
												//freigeben.
			if (xSemaphoreTake(scaleAccess, portMAX_DELAY) == pdTRUE){
				sendTo(WAITSTATION_STOP, STOP_INACTIVE);
			}
			else puts("Waage konnte nicht belegt werden.\n");
			break;

		default:
			vTaskDelay(5);	/*allow task switch*/
			break;
		}


	}
}