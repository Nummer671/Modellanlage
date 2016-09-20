///////////////////////////////////////////////////////////
//  Multiple Wait Station.h
//  Implementation of the Class Multiple Wait Station
//  Created on:      27-Aug-2016 18:54:49
//  Original author: Patrick Eichenseer
///////////////////////////////////////////////////////////

#if !defined(EA_F9402815_CA9B_4ece_9F69_09507CABB3E4__INCLUDED_)
#define EA_F9402815_CA9B_4ece_9F69_09507CABB3E4__INCLUDED_

#include "Stop_Place_Single_Direction.h"

class Multiple Wait Station
{

public:
	Multiple Wait Station();
	virtual ~Multiple Wait Station();
	Stop_Place_Single_Direction *Stop place;

	void setStationOccupied();

private:
	char StationOccupied;

};
#endif // !defined(EA_F9402815_CA9B_4ece_9F69_09507CABB3E4__INCLUDED_)
