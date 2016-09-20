///////////////////////////////////////////////////////////
//  Single Stop Place Station.h
//  Implementation of the Class Single Stop Place Station
//  Created on:      27-Aug-2016 18:55:36
//  Original author: Patrick Eichenseer
///////////////////////////////////////////////////////////

#if !defined(EA_F8ED2C9F_4262_45a4_B6B1_503438233BC2__INCLUDED_)
#define EA_F8ED2C9F_4262_45a4_B6B1_503438233BC2__INCLUDED_

#include "Stop_Place_Single_Direction.h"

class Single Stop Place Station
{

public:
	Single Stop Place Station();
	virtual ~Single Stop Place Station();
	Stop_Place_Single_Direction *StopPlace;

	bool setStationOccupied();

private:
	bool StationOccupied;

};
#endif // !defined(EA_F8ED2C9F_4262_45a4_B6B1_503438233BC2__INCLUDED_)
