/**********************************************************
File: ComCodes.h
Author: Severin Wagner
Description:
In dieser Datei werden die symbolischen Namen für die 
Kommunikation mit dem Modellbahnsimulator festgelegt
**********************************************************/

/*******Bereiche**********/
#define START_AREA_STOP		0
#define START_AREA_ENTRY	0
#define START_AREA_EXIT		1

#define SWITCH_LOADSTATION 	10
#define LOAD_PLACE_1_STOP	1
#define LOAD_PLACE_1_ENTRY	2
#define LOAD_PLACE_1_EXIT	3

#define LOAD_PLACE_2_STOP	2
#define LOAD_PLACE_2_ENTRY	4
#define LOAD_PLACE_2_EXIT	5

#define SCALE_UNLOAD_STOP	3
#define SCALE_UNLOAD_ENTRY	6
#define SCALE_UNLOAD_EXIT	7

#define SWITCH_UNLOAD_1 11
#define UNLOAD_PLACE_1_STOP 4
#define UNLOAD_PLACE_1_ENTRY 8
#define UNLOAD_PLACE_1_EXIT 9

#define UNLOAD_PLACE_2_STOP 5
#define UNLOAD_PLACE_2_ENTRY 10
#define UNLOAD_PLACE_2_EXIT 11

#define SWITCH_UNLOAD_2 12
#define UNLOAD_PLACE_3_STOP 6
#define UNLOAD_PLACE_3_ENTRY 12
#define UNLOAD_PLACE_3_EXIT 13

#define WAITSTATION_STOP 7
#define WAITSTATION_ENTRY 14
#define WAITSTATION_EXIT 15

#define SCALE_START_STOP 3
#define SCALE_START_ENTRY 16
#define SCALE_START_EXIT 17


/*******Aktoren************/

//Richtung der Weichen
#define LOAD_PLACE_1 1
#define LOAD_PLACE_2 2

#define UNLOAD_PLACE_1 1
#define UNLOAD_STRAIGHT_1 2
#define UNLOAD_PLACE_2 3

#define UNLOAD_PLACE_3 1
#define UNLOAD_STRAIGHT_2 2

//STOP-Aktoren
#define STOP_ACTIVE 1
#define STOP_INACTIVE 0

/*******Aktivitäten**********/
#define SCALE_START 20
#define SCALE_END 20

#define LOADING_1_START 21
#define LOADING_1_END 21

#define LOADING_2_START 22
#define LOADING_2_END 22

#define UNLOADING_1_START 23
#define UNLOADING_1_END 23

#define UNLOADING_2_START 24
#define UNLOADING_2_END 24

#define UNLOADING_3_START 25
#define UNLOADING_3_END 25