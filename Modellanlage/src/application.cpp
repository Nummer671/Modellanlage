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

using namespace std;

void scalefoo(void* pvParameters);

/******* Insert your code in this task **********************************************************/
extern "C" void taskApplication(void *pvParameters)
{
	(void*)pvParameters; // Prevent unused warning

	int myBuffer;
	const char scale[]			= "scale";
	const char waitStation[]	= "waitStation";
	const char startArea[]		= "startArea";
	const char loadStation[]	= "loadStation";
	const char unloadStation[]	= "unloadStation";
	
	vTaskPrioritySet(NULL, tskIDLE_PRIORITY + 5);

	//Test123
	//Test Kommentar
	xTaskCreate(scalefoo, scale, configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 4, NULL);

	// Start des Steuerprogramms
	while (true)
	{

		xQueueReceive(initSystem(), &myBuffer, portMAX_DELAY);

		//Überprüfen, ob die Simulation erfolgreich gestartet wurde
		if (myBuffer == 30){
			printf("Simulation erfolgreich gestartet");
		}
		else printf("Fehler beim Starten der Simulation");
		vTaskDelay(1000);
	}
			

}
