/***************************************************
Module name: StartAreaFoo.cpp
First written on 18.09.16 by Severin Wagner.
Module Description:
Function for the Start Area of the Modellanlage.
Die Funktion gibt bei Erfolg 0 und bei einem Fehler
-1 zurück.
***************************************************/
/* Include section */
#include <stdio.h>
#include <FreeRTOS.h>
#include <SimEventCodes.h>
#include <iostream>
using namespace std;
/***************************************************/
/* Defines section
Add all #defines here
***************************************************/
/* Function Prototype Section
Add prototypes for all *** called by this
module, with the exception of runtime routines.
***************************************************/

int16_t StartAreaFoo(int16_t status, bool statusRoadway){
	return status;
}