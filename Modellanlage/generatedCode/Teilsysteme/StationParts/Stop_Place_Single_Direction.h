///////////////////////////////////////////////////////////
//  Stop_Place_Single_Direction.h
//  Implementation of the Class Stop_Place_Single_Direction
//  Created on:      27-Aug-2016 18:54:20
//  Original author: Patrick Eichenseer
///////////////////////////////////////////////////////////

#if !defined(EA_C6D02E0A_F8F4_40ab_9969_1E0A36FF5292__INCLUDED_)
#define EA_C6D02E0A_F8F4_40ab_9969_1E0A36FF5292__INCLUDED_

#include "\Basiselemente\PresenceDetector.h"
#include "\Basiselemente\StopActor.h"

class Stop_Place_Single_Direction
{

public:
	Stop_Place_Single_Direction();
	virtual ~Stop_Place_Single_Direction();
	Presence Detector *m_Presence Detector;
	Stop Actor *m_Stop Actor;

	int receiveValueEntrySensor();
	int receiveValueExitSensor();
	bool sendStopSignal();
	bool setPlaceOccupied();

private:
	bool place_occupied;

};
#endif // !defined(EA_C6D02E0A_F8F4_40ab_9969_1E0A36FF5292__INCLUDED_)
