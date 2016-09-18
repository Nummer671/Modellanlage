#pragma once

enum state {OCCUPIED, FREE};

class WaitStation
{
public:
	WaitStation();
	~WaitStation();

	bool getStatus();
	void setStatus(state status);

private:
	enum state stationStatus;
};

