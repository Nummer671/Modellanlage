///////////////////////////////////////////////////////////
//  Single Wait Station.h
//  Implementation of the Class Single Wait Station
//  Created on:      27-Aug-2016 18:55:57
//  Original author: Severin Wagner
///////////////////////////////////////////////////////////

#if !defined(EA_956B394B_BF38_402d_9EFD_7B78056AEBC2__INCLUDED_)
#define EA_956B394B_BF38_402d_9EFD_7B78056AEBC2__INCLUDED_

#include "Presence Detector.h"
#include "Stop Actor.h"

class Single Wait Station
{

public:
	Single Wait Station();
	virtual ~Single Wait Station();
	Presence Detector *WaitStationEntryDetector;
	Stop Actor *WaitStationStopActor;

};
#endif // !defined(EA_956B394B_BF38_402d_9EFD_7B78056AEBC2__INCLUDED_)
