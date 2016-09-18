#include "WaitStation.h"


WaitStation::WaitStation()
{
}


WaitStation::~WaitStation()
{
}

bool WaitStation::getStatus(){
	return stationStatus;
}

void WaitStation::setStatus(state status){
	stationStatus = status;
}