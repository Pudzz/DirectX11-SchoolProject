#pragma once
#include <Windows.h>

class Timer
{
public:
	Timer();
	~Timer();
	
	float DeltaTime()const;

	void Reset();  // Reset values before message loop.
	void Frame();  // Call every frame.

private:	
	double secondsPerCount;
	double deltaTime;

	__int64 baseTime;
	__int64 prevTime;
	__int64 currentTime;
};