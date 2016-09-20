///////////////////////////////////////////////////////////
//  Both Direction Single Wait Station.h
//  Implementation of the Class Both Direction Single Wait Station
//  Created on:      27-Aug-2016 18:54:37
//  Original author: Patrick Eichenseer
///////////////////////////////////////////////////////////

#if !defined(EA_1B02609A_B0EE_4217_9DA4_2A40B8EEC8C9__INCLUDED_)
#define EA_1B02609A_B0EE_4217_9DA4_2A40B8EEC8C9__INCLUDED_

#include "StopPlaceBothDirection.h"

class Both Direction Single Wait Station
{

public:
	Both Direction Single Wait Station();
	virtual ~Both Direction Single Wait Station();
	StopPlaceBothDirection *Stop Place Both Direction;

	bool setStationOccupied();
	void weightProcessActive();

private:
	bool StationOccupied;

};
#endif // !defined(EA_1B02609A_B0EE_4217_9DA4_2A40B8EEC8C9__INCLUDED_)
