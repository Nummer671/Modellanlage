///////////////////////////////////////////////////////////
//  StopPlaceBothDirection.h
//  Implementation of the Class StopPlaceBothDirection
//  Created on:      27-Aug-2016 18:53:44
//  Original author: Severin Wagner
///////////////////////////////////////////////////////////

#if !defined(EA_F5B5B488_556E_414d_874B_0DB8AE863F2B__INCLUDED_)
#define EA_F5B5B488_556E_414d_874B_0DB8AE863F2B__INCLUDED_

#include "Presence Detector.h"
#include "Stop Actor.h"

class StopPlaceBothDirection
{

public:
	StopPlaceBothDirection();
	virtual ~StopPlaceBothDirection();
	Presence Detector *ExitDetectorLeft;
	Presence Detector *ExitDetectorRight;
	Stop Actor *StopActorLeft;
	Presence Detector *EntryDetectorLeft;
	Presence Detector *EntryDetectorRight;
	Stop Actor *StopActorRight;

	void setPlaceOccupied(bool place_Occupied);

private:
	bool place_occupied;

};
#endif // !defined(EA_F5B5B488_556E_414d_874B_0DB8AE863F2B__INCLUDED_)
