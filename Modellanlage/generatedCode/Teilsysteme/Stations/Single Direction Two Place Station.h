///////////////////////////////////////////////////////////
//  Single Direction Two Place Station.h
//  Implementation of the Class Single Direction Two Place Station
//  Created on:      27-Aug-2016 18:55:21
//  Original author: Patrick Eichenseer
///////////////////////////////////////////////////////////

#if !defined(EA_40ADC1C7_FC5B_48ec_ACE3_A5EB024671BD__INCLUDED_)
#define EA_40ADC1C7_FC5B_48ec_ACE3_A5EB024671BD__INCLUDED_

#include "turnout 2-way.h"
#include "Stop_Place_Single_Direction.h"

class Single Direction Two Place Station
{

public:
	Single Direction Two Place Station();
	virtual ~Single Direction Two Place Station();
	turnout 2-way *Turnout;
	Stop_Place_Single_Direction *Stop Place 2;
	Stop_Place_Single_Direction *Stop Place 1;

	bool loadingActivPlace1();
	bool loadingActivPlace2();
	void setPlaceDirection();
	bool setStationOccupied();

private:
	bool StationOccupied;

};
#endif // !defined(EA_40ADC1C7_FC5B_48ec_ACE3_A5EB024671BD__INCLUDED_)
