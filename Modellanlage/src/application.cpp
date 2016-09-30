/**************************************************************************************************
Module:			application.cpp
Purpose:		This template provides a FreeRTOS Windows Simulation
System:			Visual Studio / Windows - FreeRTOS Simulator
Date:			2016-03-11
Version:		2.1.0
Description:	Dieses Modul bildet die Steuerungstasks der Modellanlage ab. Die taskApplication-Task
				ist die Haupttask, die die Anlage initialisiert und die Kommunikation mit dem
				Simulator übernimmt. Die Ereignisse werden von dieser Task an die anderen Tasks
				weitergeleitet.
**************************************************************************************************/
#include <FreeRTOS.h>
#include <stdio.h>
#include <iostream>
#include "task.h"
#include "util\bprint.h"
#include "util\simCommunication.h"
#include "SimEventCodes.h"
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
ThreePlaceWaitStation UnloadingStation;

/******* Insert your code in this task **********************************************************/
extern "C" void taskApplication(void *pvParameters)
{
	(void*)pvParameters; // Compiler Warnung vermeiden

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
	if (startQueue == 0) printf("startQueue konnte nicht erzeugt werden.\n");

	loadQueue = xQueueCreate(25, sizeof(int));
	if (loadQueue == 0) printf("loadQueue konnte nicht erzeugt werden.\n");

	scaleQueue = xQueueCreate(10, sizeof(int));
	if (scaleQueue == 0) printf("scaleQueue konnte nicht erzeugt werden.\n");

	unloadQueue = xQueueCreate(25, sizeof(int));
	if (unloadQueue == 0) printf("unloadQueue konnte nicht erzeugt werden.\n");

	waitQueue = xQueueCreate(10, sizeof(int));
	if (waitQueue == 0) printf("waitQueue konnte nicht erzeugt werden.\n");

	//Initialisieren des Simulators
	//initSystem() gibt die Message Queue zurück, auf der die Nachrichten des Simulators empfangen werden.
	statusQueue = initSystem();

	/*Initialsieren der Stationen*/
	if (UnloadingStation.stationInit() == 0){
		printf("Entladestation erfolgreich initialisiert.\n");
	}
	else puts("Entladestation konnte nicht initialisiert werden.\n");

	vTaskDelay(4000); /*Wartezeit für die Initialisierung des Systems*/

	//Erzeugen der Semaphoren
	loadingStationCount = xSemaphoreCreateCounting(2, 2);
	if (loadingStationCount == NULL) printf("Semaphore loadingStationCount konnte nicht erzeugt werden.\n");

	loadingStationAccess = xSemaphoreCreateBinary();
	if (loadingStationAccess == NULL) printf("Semaphore loadingStationAccess konnte nicht erzeugt werden.\n");

	scaleAccess = xSemaphoreCreateBinary();
	if (scaleAccess == NULL) printf("Semaphore scaleAccess konnte nicht erzeugt werden.\n");

	unloadingStationCount = xSemaphoreCreateCounting(3, 3);
	if (unloadingStationCount == NULL) printf("Semaphore unloadingStationCount konnte nicht erzeugt werden.\n");

	unloadingStationAccess = xSemaphoreCreateBinary();
	if (unloadingStationAccess == NULL) printf("Semaphore unloadingStationAccess konnte nicht erzeugt werden.\n");

	waitStationAccess = xSemaphoreCreateBinary();
	if (waitStationAccess == NULL) printf("Semaphore waitStationAccess konnte nicht erzeugt werden.\n");

	unloadSwitch2 = xSemaphoreCreateBinary();
	if (unloadSwitch2 == NULL) printf("Semaphore unloadSwitch2 konnte nicht erzeugt werden.\n");

	//Prüfen, ob die Simulation erfolgreich gestartet wurde.
	xQueueReceive(statusQueue, &buffer, portMAX_DELAY);

	if (buffer == 30) {
		printf("Simulation erfolgreich gestartet. \n");
		/*Steuerungstasks anlegen*/
		if (xTaskCreate(startAreaFunc, startArea, configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &startArea_t) == pdFALSE){
			printf("Task für den Startbereich konnte nicht erzeugt werden.\n");
			vTaskDelay(portMAX_DELAY);	/*Anhalten, falls die Task nicht erzeugt werden konnte.*/
		}
		if (xTaskCreate(loadingStationFunc, loadStation, configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &loadStation_t) == pdFALSE){
			printf("Task für die Beladestation konnte nicht erzeugt werden.\n");
			vTaskDelay(portMAX_DELAY);	/*Anhalten, falls die Task nicht erzeugt werden konnte.*/
		}
		if (xTaskCreate(scaleFunc, scale, configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 5, &scale_t) == pdFALSE){
			printf("Task für die Waage konnte nicht erzeugt werden.\n");
			vTaskDelay(portMAX_DELAY);	/*Anhalten, falls die Task nicht erzeugt werden konnte.*/
		}
		if (xTaskCreate(unloadingStationFunc, unloadStation, configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, &unloadStation_t) == pdFALSE){
			printf("Task für die Entladestation konnte nicht erzeugt werden.\n");
			vTaskDelay(portMAX_DELAY);	/*Anhalten, falls die Task nicht erzeugt werden konnte.*/
		}
		if (xTaskCreate(waitStationFunc, waitStation, configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 4, &waitStation_t) == pdFALSE){
			printf("Task für den Wartebereich konnte nicht erzeugt werden.\n");
			vTaskDelay(portMAX_DELAY);	/*Anhalten, falls die Task nicht erzeugt werden konnte.*/
		}
	}
	else puts("Simulation konnte nicht erfolgreich gestartet werden. \n");

	/*Start der Taskfunktion*/
	while (true)
	{
		if (xQueueReceive(statusQueue, &buffer, portMAX_DELAY) == pdTRUE){

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
			if (xQueueSend(startQueue, &startQueueBuffer, 10) == pdFALSE){
				printf("Senden an startQueue fehlgeschlagen.\n");
			}
			if (xQueueSend(loadQueue, &loadQueueBuffer, 10) == pdFALSE){
				printf("Senden an loadQueue fehlgeschlagen.\n");
			}
			if (xQueueSend(scaleQueue, &scaleQueueBuffer, 10) == pdFALSE){
				printf("Senden an scaleQueue fehlgeschlagen.\n");
			}
			if (xQueueSend(unloadQueue, &unloadQueueBuffer, 10) == pdFALSE){
				printf("Senden an unloadQueue fehlgeschlagen.\n");
			}
			if (xQueueSend(waitQueue, &waitQueueBuffer, 10) == pdFALSE){
				printf("Senden an waitQueue fehlgeschlagen.\n");
			}

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
				if (xSemaphoreGive(loadingStationCount) == pdFALSE){
					printf("Freigeben von loadingStationCount fehlgeschlagen.\n");
				}
				break;

			case LOAD_PLACE_2_EXIT:
				sendTo(LOAD_PLACE_2_STOP, STOP_ACTIVE);
				sendTo(SWITCH_LOAD_PLACE, LOAD_PLACE_2);
				if (xSemaphoreGive(loadingStationCount) == pdFALSE){
					printf("Freigeben von loadingStationCount fehlgeschlagen.\n");
				}
				break;

			case UNLOAD_PLACE_1_EXIT:
				if (xSemaphoreGive(unloadingStationCount) == pdFALSE) printf("Freigeben von unloadingStationCount fehlgeschlagen.\n");
				if (UnloadingStation.stationRelease(0) == -1) puts("Ein Fehler beim Freigeben von Entladeplatz 1 ist aufgetreten.\n");
				sendTo(UNLOAD_PLACE_1_STOP, STOP_ACTIVE);
				break;

			case UNLOAD_PLACE_2_EXIT:
				if (xSemaphoreGive(unloadingStationCount) == pdFALSE) printf("Freigeben von unloadingStationCount fehlgeschlagen.\n");
				if (UnloadingStation.stationRelease(1) == -1) puts("Ein Fehler beim Freigeben von Entladeplatz 2 ist aufgetreten.\n");
				sendTo(UNLOAD_PLACE_2_STOP, STOP_ACTIVE);
				break;

			case UNLOAD_PLACE_3_EXIT:
				if (xSemaphoreGive(unloadingStationCount) == pdFALSE) printf("Freigeben von unloadingStationCount fehlgeschlagen.\n");
				if (UnloadingStation.stationRelease(2) == -1) puts("Ein Fehler beim Freigeben von Entladeplatz 3 ist aufgetreten.\n");
				sendTo(UNLOAD_PLACE_3_STOP, STOP_ACTIVE);
				break;

			case SCALE_START_EXIT:
				sendTo(SCALE_START_STOP, STOP_ACTIVE);
				if (xSemaphoreGive(scaleAccess) == pdFALSE) printf("Freigeben von scaleAccess fehlgeschlagen.\n");
				break;

			case SCALE_UNLOAD_EXIT:
				sendTo(SCALE_UNLOAD_STOP, STOP_ACTIVE);
				if (xSemaphoreGive(scaleAccess) == pdFALSE) printf("Freigeben von scaleAccess fehlgeschlagen.\n");
				break;

			case WAITSTATION_EXIT:
				sendTo(WAITSTATION_STOP, STOP_ACTIVE);
				if (xSemaphoreGive(waitStationAccess) == pdFALSE) printf("Freigeben von waitStationAccess fehlgeschlagen.\n");
				break;

			case ENDSEQUENZ:
				/*Sobald der Befehl zum Beenden der Simulation empfangen wurde, wird der
				Stopp des Startbereichs aktiviert und die Starttask suspendiert, so dass
				keine Fahrzeuge mehr in die Anlage einfahren können. Alle anderen 
				Fahrzeuge in der Anlage beenden noch ihren Zyklus.*/
				sendTo(START_AREA_STOP, STOP_ACTIVE);
				vTaskSuspend(startArea_t);
				printf("Endsequenz eingeleitet.\n");
				break;

			default:
				vTaskDelay(1);	/*Taskwechsel zulassen*/
				break;
			}
		}
		else puts("Application: Keine Daten.\n");
	}
}

void startAreaFunc(void* pvParameters){
	/*Variablen*/
	BaseType_t statusRoadway;
	int16_t buffer;

	(void*)pvParameters; // Compiler Warnung vermeiden

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
					printf("Startbereich: Strecke belegt.\n");
				}
				else{
					if (xSemaphoreTake(loadingStationCount, portMAX_DELAY) == pdTRUE){
						sendTo(START_AREA_STOP, STOP_INACTIVE);
					}
					else printf("Startbereich: Keine Station frei.\n");
				}
				break;

			default:
				vTaskDelay(1); /*Taskwechsel zulassen*/
				break;
			}
		}

	}
}

void loadingStationFunc(void* pvParameters){
	/*Variablen*/
	int16_t buffer;

	(void*)pvParameters; // Compiler Warnung vermeiden

	/*Initialisierung der Semaphoren*/
	if (xSemaphoreGive(loadingStationAccess) == pdFALSE){
		printf("Beladestation: Fehler beim Freigeben der Semaphore.\n");
	}


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
				if (xSemaphoreGive(loadingStationAccess) == pdFALSE){
					printf("Beladestation: Fehler beim Freigeben der Semaphore.\n");
				}
				break;

			case LOADING_1_END:
				/*Prüfen, ob die Waage befahren werden kann und ob ein Entladeplatz frei ist.
				Die Prüfung dient dazu, dass die Waage immer frei werden kann und kein Deadlock entsteht.*/
				if (xSemaphoreTake(unloadingStationCount, portMAX_DELAY) == pdTRUE) {
					if (xSemaphoreTake(scaleAccess, portMAX_DELAY) == pdTRUE) sendTo(LOAD_PLACE_1_STOP, STOP_INACTIVE);
					else printf("Beladestation: Fehler bei Zugriff auf Semaphore.\n");
				}
				else printf("Beladestation: Fehler bei Zugriff auf Semaphore.\n");
				break;

			case LOAD_PLACE_2_ENTRY:
				sendTo(SWITCH_LOAD_PLACE, LOAD_PLACE_1);
				sendTo(LOADING_2_START, LOADING_2_START);
				if (xSemaphoreGive(loadingStationAccess) == pdFALSE){
					printf("Beladestation: Fehler beim Freigeben der Semaphore.\n");
				}
				break;

			case LOADING_2_END:
				/*Prüfen, ob die Waage befahren werden kann und ob ein Entladeplatz frei ist.
				Die Prüfung dient dazu, dass die Waage immer frei werden kann und kein Deadlock entsteht.*/
				if (xSemaphoreTake(unloadingStationCount, portMAX_DELAY) == pdTRUE) {
					if (xSemaphoreTake(scaleAccess, portMAX_DELAY) == pdTRUE)  sendTo(LOAD_PLACE_2_STOP, STOP_INACTIVE);
					else printf("Beladestation: Fehler bei Zugriff auf Semaphore.\n");
				}
				else printf("Beladestation: Fehler bei Zugriff auf Semaphore.\n");
				break;

			default:
				vTaskDelay(1);	/*Taskwechsel zulassen*/
				break;
			}
		}
	}
}

void scaleFunc(void* pvParameters){
	/*Variablen*/
	bool directionUnload = true;	/*Merker für die Richtung des LKWs*/
	int16_t buffer;
	int16_t rc;

	(void*)pvParameters; // Compiler Warnung vermeiden

	/*Initialisieren der Semaphoren*/
	if (xSemaphoreGive(scaleAccess) == pdFALSE){
		printf("Waage: Fehler beim Freigeben der Semaphore.\n");
	}

	//Endlosschleife für die Taskfunktion
	while (true)
	{
		/*Task wartet bei jedem Schleifendurchlauf auf den Empfang eines neuen Ereignisses*/
		if (xQueueReceive(scaleQueue, &buffer, portMAX_DELAY) == pdFALSE){
			printf("Waage: Message Queue konnte nicht gelesen werden.\n");
		}
		else
		{
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
						/*Weiche auf den ersten Entladeplatz einstellen
						und Fahrzeug starten.*/
						sendTo(SWITCH_UNLOAD_1, UNLOAD_PLACE_1);
						sendTo(SCALE_START_STOP, STOP_INACTIVE);
					}
					else if (rc == 1){
						/*Weiche auf den zweiten Entladeplatz einstellen
						und Fahrzeug starten.*/
						sendTo(SWITCH_UNLOAD_1, UNLOAD_PLACE_2);
						sendTo(SCALE_START_STOP, STOP_INACTIVE);
					}
					else if (rc == 2){
						if (xSemaphoreTake(unloadSwitch2, portMAX_DELAY) == pdTRUE){
							/*Weichen auf den driten Entladeplatz einstellen
							und Fahrzeug starten.*/
							sendTo(SWITCH_UNLOAD_2, UNLOAD_PLACE_3);
							sendTo(SWITCH_UNLOAD_1, UNLOAD_STRAIGHT_1);
							sendTo(SCALE_START_STOP, STOP_INACTIVE);
						}
						else printf("Waage: Fehler bei Zugriff auf Semaphore unloadSwitch2.\n");
					}
					else puts("Waage: Ein Fehler beim Reservieren des Entladeplatzes ist aufgetreten.\n");
				}
				break;

				/*Prüfen, ob ein Fahrzeug auf den dritten Entladeplatz gefahren ist,
				um die Semaphore für die Weiche freizugeben.*/
			case UNLOAD_PLACE_3_ENTRY:
				if (xSemaphoreGive(unloadSwitch2) == pdFALSE){
					printf("Waage: Fehler beim Freigeben der Semaphore.\n");
				}
				break;

			case SCALE_UNLOAD_ENTRY:
				directionUnload = true;
				sendTo(SCALE_START, SCALE_START);
				break;

			default:
				vTaskDelay(1); /*Taskwechsel zulassen*/
				break;
			}
		}
	}
}

void unloadingStationFunc(void* pvParameters){
	/*Variablen*/
	int16_t buffer;

	(void*)pvParameters; // Compiler Warnung vermeiden

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
			vTaskDelay(1);	/*Taskwechsel zulassen*/
			break;
		}
	}
}

void waitStationFunc(void* pvParameters){
	/*Variablen*/
	int16_t buffer;

	(void*)pvParameters; // Compiler Warnung vermeiden

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
			vTaskDelay(1);	/*Taskwechsel zulassen*/
			break;
		}
	}
}