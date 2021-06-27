#include "Timer.h"

Timer::Timer() {
	secondsPerCount = 0.0;
	deltaTime = -1.0f;
	baseTime = 0;
	prevTime = 0;
	currentTime = 0;
	
	__int64 countsPerSecond;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSecond);
	secondsPerCount = 1.0 / (double)countsPerSecond;
}

Timer::~Timer()
{
}

float Timer::DeltaTime() const
{
	return (float)deltaTime;
}

void Timer::Reset()
{
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

	baseTime = currTime;
	prevTime = currTime;
}

void Timer::Frame()
{
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	currentTime = currTime;

	// Time difference between this frame and the previous.
	deltaTime = (currentTime - prevTime) * secondsPerCount;

	// Current time is the previous time for next frame..
	prevTime = currentTime;
}
