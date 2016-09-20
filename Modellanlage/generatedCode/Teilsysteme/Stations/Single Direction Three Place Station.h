///////////////////////////////////////////////////////////
//  Single Direction Three Place Station.h
//  Implementation of the Class Single Direction Three Place Station
//  Created on:      27-Aug-2016 18:55:02
//  Original author: Patrick Eichenseer
///////////////////////////////////////////////////////////

#if !defined(EA_7906EA55_6C7C_4706_8B45_F24928D159E7__INCLUDED_)
#define EA_7906EA55_6C7C_4706_8B45_F24928D159E7__INCLUDED_

#include "Stop_Place_Single_Direction.h"
#include "turnout 2-way.h"
#include "turnout 3-way.h"

class Single Direction Three Place Station
{

public:
	Single Direction Three Place Station();
	virtual ~Single Direction Three Place Station();
	Stop_Place_Single_Direction *StopPlace1;
	Stop_Place_Single_Direction *StopPlace3;
	turnout 2-way *Turnout2Way;
	turnout 3-way *Turnout3Way;
	Stop_Place_Single_Direction *StopPlace2;

	void setPlaceDirectionTournout2Way();
	void setPlaceDirectionTournout3Way();
	bool setStationOccupied();
	void unloadingActivePlace1();
	void unloadingActivePlace2();
	void unloadingActivePlace3();

private:
	bool StationOccupied;

};
#endif // !defined(EA_7906EA55_6C7C_4706_8B45_F24928D159E7__INCLUDED_)
