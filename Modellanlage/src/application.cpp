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
void startAreaFunc(void* pvParameters);
void loadingStationFunc(void* pvParameters);
void scaleFunc(void* pvParameters);

//Queues
xQueueHandle statusQueue;
xQueueHandle startQueue;
xQueueHandle loadQueue;
<<<<<<< HEAD
xQueueHandle scaleQueue;
=======
>>>>>>> refs/remotes/origin/master

//Semaphores
SemaphoreHandle_t loadingStationCount;
SemaphoreHandle_t loadingStationAccess;
SemaphoreHandle_t scaleAccess;
<<<<<<< HEAD
=======

>>>>>>> refs/remotes/origin/master

//TaskHandle
TaskHandle_t startArea_t;
TaskHandle_t loadStation_t;
TaskHandle_t scale_t;

<<<<<<< HEAD
//Stationen der Modellanlage
=======
//Station der Modellanlage
>>>>>>> refs/remotes/origin/master
StartArea Start1;
LoadingStation Load1;

/******* Insert your code in this task **********************************************************/
extern "C" void taskApplication(void *pvParameters)
{
	(void*)pvParameters; // Prevent unused warning

	//Variablen
	int myBuffer;
<<<<<<< HEAD
	void* pvMyBuffer = &myBuffer;	//Nachrichtenbuffer mit Typ-Cast
	int startQueueBuffer;
	int loadQueueBuffer;
	int scaleQueueBuffer;
=======
	void* pvMyBuffer = &myBuffer;
	int startbuffer;
	int loadbuffer;
>>>>>>> refs/remotes/origin/master

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

	//Initialsieren der Stationen
	//Load1.stationInit();
	Load1.setRoadwayStat(true);
	

	xQueueReceive(statusQueue, &myBuffer, portMAX_DELAY);
	if (myBuffer == 30) {
		printf("Simulation erfolgreich gestartet. \n");
<<<<<<< HEAD
		xTaskCreate(startAreaFunc, startArea, configMINIMAL_STACK_SIZE, pvMyBuffer, tskIDLE_PRIORITY + 2, &startArea_t);
		xTaskCreate(loadingStationFunc, loadStation, configMINIMAL_STACK_SIZE, pvMyBuffer, tskIDLE_PRIORITY + 2, &loadStation_t);
		xTaskCreate(scaleFunc, scale, configMINIMAL_STACK_SIZE, pvMyBuffer, tskIDLE_PRIORITY + 3, &scale_t);

		//Erzeugen der Message Queues
		startQueue = xQueueCreate(15, sizeof(int));
		loadQueue = xQueueCreate(15, sizeof(int));
		scaleQueue = xQueueCreate(10, sizeof(int));
=======
		xTaskCreate(startAreaFunc, startArea, configMINIMAL_STACK_SIZE, pvMyBuffer, tskIDLE_PRIORITY + 1, &startArea_t);
		xTaskCreate(loadingStationFunc, loadStation, configMINIMAL_STACK_SIZE, pvMyBuffer, tskIDLE_PRIORITY + 2, &loadStation_t);

		//Erzeugen der einzelnen Message Queues
		startQueue = xQueueCreate(10, sizeof(int));
		loadQueue = xQueueCreate(10, sizeof(int));
>>>>>>> refs/remotes/origin/master
	}
	else puts("Simulation konnte nicht gestartet werden. \n");
	
	//vTaskPrioritySet(NULL, tskIDLE_PRIORITY + 4);
	// Start des Steuerprogramms
	while (true)
	{
		if (xQueueReceive(statusQueue, &myBuffer, portMAX_DELAY)){
			printf("Daten %i empfangen. \n", myBuffer);
<<<<<<< HEAD
			startQueueBuffer = myBuffer;
			loadQueueBuffer = myBuffer;
			scaleQueueBuffer = myBuffer;

			xQueueSend(startQueue, &startQueueBuffer, portMAX_DELAY);
			xQueueSend(loadQueue, &loadQueueBuffer, portMAX_DELAY);
			xQueueSend(scaleQueue, &scaleQueueBuffer, portMAX_DELAY);
=======
			startbuffer = myBuffer;
			loadbuffer = myBuffer;
			xQueueSend(startQueue, &startbuffer, portMAX_DELAY);
			xQueueSend(loadQueue, &loadbuffer, portMAX_DELAY);
>>>>>>> refs/remotes/origin/master
		}
		else puts("keine Daten.\n");
	}
			

}

void startAreaFunc(void* pvParameters){
<<<<<<< HEAD
	int16_t status = -1;
	int16_t error = -1;
	bool vehicleStarted = false;
	bool statusRoadway = true;
	bool stopActor = true;

	int16_t statusLoadPlace = 0;
	int buffer;
=======
	BaseType_t status = -1;
	int error = -1;
	int buffer;
	int16_t error = -1;
	bool vehicleStarted = false;
	bool roadwayStat = false;
	bool stopActor = true;
>>>>>>> refs/remotes/origin/master

	while (true)
	{
		//status = *(int*)pvParameters;
<<<<<<< HEAD
		
		printf("Task 1 gestartet.\n");
		xQueueReceive(startQueue, &buffer, portMAX_DELAY); /*Task wartet bei jedem Schleifendurchlauf auf den Empfang eines neuen Ereignisses*/
		
		switch (buffer){
		case START_AREA_ENTRY:
			//Überprüfen, ob die Strecke frei ist.
			statusRoadway = xSemaphoreTake(loadingStationAccess, portMAX_DELAY);
			if (statusRoadway == pdFALSE) {
				printf("Strecke belegt.\n");
				vTaskDelay(10);
			}
			else{
				statusLoadPlace = Load1.loadPlaceReserve();
				if (statusLoadPlace == 0 || statusLoadPlace == 1){
					//statusRoadway = false;
					Load1.setRoadwayStat(false);
					printf("Fahrzeug gestartet.\n");
					sendTo(START_AREA_STOP, STOP_INACTIVE);
					stopActor = false;
					//vTaskDelay(5);		//Verzögerung, damit der LKW ausreichend Zeit zum Starten hat.
=======
		status = xQueueReceive(startQueue, &buffer, portMAX_DELAY);
		printf("Startbereich gestartet.\n");

		//Überprüfen, ob die Strecke frei ist.
		roadwayStat = Load1.getRoadwayStat();

		switch (status){
		case START_AREA_ENTRY:
			if (roadwayStat == false) {
				printf("Strecke belegt.\n");
				vTaskDelay(100);
			}
			else{
				error = Load1.stationReserve();
				if (error == -1) printf("Es konnte keine Station reserviert werden.\n");
				else{
					sendTo(START_AREA_STOP, STOP_INACTIVE);
					Load1.setRoadwayStat(false);
					stopActor = false;
					vTaskDelay(10);		//Verzögerung, damit der LKW ausreichend Zeit zum Starten hat.
>>>>>>> refs/remotes/origin/master
				}
				else printf("Keine Station frei.\n");
				//error = Load1.stationReserve();
				//if (error == -1) printf("Es konnte keine Station reserviert werden.\n");
			}
			break;

		case START_AREA_EXIT:
<<<<<<< HEAD
			if (stopActor == false){
				printf("Startbereich verlassen.\n");
				stopActor = true;
				sendTo(START_AREA_STOP, STOP_ACTIVE);
			}
			else vTaskDelay(10);
			break;

		//case LOAD_PLACE_1_ENTRY:
		//	if (loadPlace1){
		//		loadPlace1 = false;
		//		Load1.setRoadwayStat(true);
		//		sendTo(SWITCH_LOAD_PLACE, LOAD_PLACE_2);
		//		printf("Weiche nach 2 gestellt.\n");
		//		sendTo(LOADING_1_START, LOADING_1_START);
		//		printf("Beladen 1 gestartet.\n");
		//		//xSemaphoreGive(loadingStationAccess);
		//		printf("Start -> Beladestation frei.\n");
		//		//vTaskDelay(10);
		//	}
		//	break;

		//case LOADING_1_END:
		//	printf("Beladen beendet.\n");
		//	sendTo(LOAD_PLACE_1_STOP, STOP_INACTIVE);
		//	break;

		//case LOAD_PLACE_1_EXIT:
		//	if (!loadPlace1){
		//		sendTo(LOAD_PLACE_1_STOP, STOP_ACTIVE);
		//		//sendTo(SWITCH_LOAD_PLACE, LOAD_PLACE_1);
		//		loadPlace1 = true;
		//		Load1.loadPlaceRelease(1);
		//		printf("Ladeplatz 1 frei.\n");
		//		//xSemaphoreGive(loadingStationCount);
		//		//printf("Zählsemaphore zurückgeben.\n");
		//		//xSemaphoreGive(loadingStationAccess);
		//		//printf("Start -> Beladestation frei.\n");
		//	}
		//	break;

		//case LOAD_PLACE_2_ENTRY:
		//	if (loadPlace2){
		//		loadPlace2 = false;
		//		Load1.setRoadwayStat(true);
		//		sendTo(SWITCH_LOAD_PLACE, LOAD_PLACE_1);
		//		printf("Weiche nach 2 gestellt.\n");
		//		sendTo(LOADING_2_START, LOADING_2_START);
		//		printf("Beladen 1 gestartet.\n");
		//		//xSemaphoreGive(loadingStationAccess);
		//		//printf("Start -> Beladestation frei.\n");
		//		//vTaskDelay(10);
		//	}
		//	break;

		//case LOADING_2_END:
		//	printf("Beladen beendet.\n");
		//	sendTo(LOAD_PLACE_2_STOP, STOP_INACTIVE);
		//	break;

		//case LOAD_PLACE_2_EXIT:
		//	if (!loadPlace2){
		//		sendTo(LOAD_PLACE_2_STOP, STOP_ACTIVE);
		//		//sendTo(SWITCH_LOAD_PLACE, LOAD_PLACE_2);
		//		loadPlace2 = true;
		//		Load1.loadPlaceRelease(0);
		//		printf("Ladeplatz 2 frei.\n");
		//		//xSemaphoreGive(loadingStationCount);
		//		//printf("Zählsemaphore zurückgeben.\n");
		//		//xSemaphoreGive(loadingStationAccess);
		//		//printf("Start -> Beladestation frei.\n");
		//	}
		//	break;

		default:
			printf("Task 1 idle.\n");
			vTaskDelay(5); /*allow Task switch*/
=======
			if (stopActor) vTaskDelay(20);
			else{
				sendTo(START_AREA_STOP, STOP_ACTIVE);
				stopActor = true;
			}
			break;

		default:
			printf("Startbereich idle.\n");
			vTaskDelay(100);
>>>>>>> refs/remotes/origin/master
			break;
		}
	}
}

void loadingStationFunc(void* pvParameters){
	//Variablen
<<<<<<< HEAD
	int status = -1;
=======
	BaseType_t status = -1;
>>>>>>> refs/remotes/origin/master
	int buffer;
	bool loadPlace1 = true;
	bool loadPlace2 = true;

	xSemaphoreGive(scaleAccess);
	//Task-Funktion
	while (true)
	{
		//status = *(int*)pvParameters;
<<<<<<< HEAD
=======
		status = xQueueReceive(loadQueue, &buffer, portMAX_DELAY);
>>>>>>> refs/remotes/origin/master
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
<<<<<<< HEAD
		else if (buffer == LOADING_1_END ){
			printf("Beladen beendet.\n");

			//Prüfen, ob die Waage befahren werden kann?
			if (xSemaphoreTake(scaleAccess, portMAX_DELAY)) {
				sendTo(LOAD_PLACE_1_STOP, STOP_INACTIVE);
				//vTaskDelay(10);
			}
			
=======
		else if (status == LOADING_1_END){
			printf("Beladen beendet.\n");

			//Prüfen, ob die Waage befahren werden kann?
			if (xSemaphoreTake(scaleAccess, 500)) {
				sendTo(LOAD_PLACE_1_STOP, STOP_INACTIVE);
				//vTaskDelay(10);
			}

>>>>>>> refs/remotes/origin/master
		}
		// Beladestation 1 verlassen?
		else if (buffer == LOAD_PLACE_1_EXIT && !loadPlace1){
			sendTo(LOAD_PLACE_1_STOP, STOP_ACTIVE);
			sendTo(SWITCH_LOAD_PLACE, LOAD_PLACE_1);
			loadPlace1 = true;
			xSemaphoreGive(loadingStationCount);
			printf("Zählsemaphore zurückgeben.\n");
			xSemaphoreGive(loadingStationAccess);
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
<<<<<<< HEAD
		else if (buffer == LOADING_2_END){

			if (xSemaphoreTake(scaleAccess, portMAX_DELAY)) {
=======
		else if (status == LOADING_2_END){

			if (xSemaphoreTake(scaleAccess, 500)) {
>>>>>>> refs/remotes/origin/master
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
			xSemaphoreGive(loadingStationAccess);
			printf("Start -> Beladestation frei.\n");
<<<<<<< HEAD
			
=======

>>>>>>> refs/remotes/origin/master
		}
		//Warten
		else {
			printf("Beladestation idle.\n");
<<<<<<< HEAD
			vTaskDelay(5);	/*allow Task switch*/
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

		else vTaskDelay(5); /*allow Task switch*/
=======
			vTaskDelay(10);
		}
>>>>>>> refs/remotes/origin/master
	}
}