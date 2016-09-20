///////////////////////////////////////////////////////////
//  Turnout3Way.h
//  Implementation of the Class turnout 3-way
//  Created on:      27-Aug-2016 18:51:51
//  Original author: Severin Wagner
///////////////////////////////////////////////////////////

#if !defined(EA_764064DF_73AD_41bf_AAEA_684372F838F9__INCLUDED_)
#define EA_764064DF_73AD_41bf_AAEA_684372F838F9__INCLUDED_

#include "Turnout3Way.h"

class Turnout3Way : public Turnout3Way
{

public:
	Turnout3Way();
	virtual ~Turnout3Way();

	int setTurnoutDirection(int TurnoutDirection);
	virtual int sendTo(int Direction, int TurnoutNumber);

private:
	int TurnoutDirection;

};
#endif // !defined(EA_764064DF_73AD_41bf_AAEA_684372F838F9__INCLUDED_)
