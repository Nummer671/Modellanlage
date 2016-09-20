///////////////////////////////////////////////////////////
//  PresenceDetector.h
//  Implementation of the Class PresenceDetector
//  Created on:      03-Jul-2016 13:40:34
//  Original author: Severin Wagner
///////////////////////////////////////////////////////////

#if !defined(EA_CB315210_191E_4aeb_998B_314B4ACD9E0C__INCLUDED_)
#define EA_CB315210_191E_4aeb_998B_314B4ACD9E0C__INCLUDED_

class PresenceDetector
{

public:
	PresenceDetector();
	virtual ~PresenceDetector();

	int getValue();

private:
	int vehicleDetected;

};
#endif // !defined(EA_CB315210_191E_4aeb_998B_314B4ACD9E0C__INCLUDED_)
