///////////////////////////////////////////////////////////
//  StopActor.h
//  Implementation of the Class Stop Actor
//  Created on:      27-Aug-2016 18:50:57
//  Original author: Severin Wagner
///////////////////////////////////////////////////////////

#if !defined(EA_F0E4EA74_67CF_43d8_A0BE_5AE3A7175B73__INCLUDED_)
#define EA_F0E4EA74_67CF_43d8_A0BE_5AE3A7175B73__INCLUDED_

#include "StopActor.h"

class StopActor : public StopActor
{

public:
	StopActor();
	virtual ~StopActor();

	void setStopSignal(bool StopSignal);
	virtual int sendTo(int StationNumber, bool Action);

private:
	int StopActorNumber;
	bool StopSignal;

};
#endif // !defined(EA_F0E4EA74_67CF_43d8_A0BE_5AE3A7175B73__INCLUDED_)
