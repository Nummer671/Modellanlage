///////////////////////////////////////////////////////////
//  turnout 2-way.h
//  Implementation of the Class turnout 2-way
//  Created on:      27-Aug-2016 18:51:39
//  Original author: Severin Wagner
///////////////////////////////////////////////////////////

#if !defined(EA_40A85E16_4467_4cba_A867_2FD3EB6929C6__INCLUDED_)
#define EA_40A85E16_4467_4cba_A867_2FD3EB6929C6__INCLUDED_

#include "Turnout2Way.h"

class Turnout2Way : public Turnout2Way
{

public:
	Turnout2Way();
	virtual ~Turnout2Way();

	int setTurnoutDirection(int TurnoutDirection);
	virtual int sendTo(int Direction, int TurnoutNumber);

private:
	int TurnoutDirection;

};
#endif // !defined(EA_40A85E16_4467_4cba_A867_2FD3EB6929C6__INCLUDED_)
