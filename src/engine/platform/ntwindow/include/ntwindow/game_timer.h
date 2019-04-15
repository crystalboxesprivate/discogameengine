#pragma once
//***************************************************************************************
// GameTimer.h by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

class GameTimer
{
public:
	GameTimer();

	float get_total_time()const;
	float get_delta_time()const;

	void reset();
	void start();
	void stop();
	void tick();

private:
	double mSecondsPerCount;
	double mDeltaTime;

	__int64 mBaseTime;
	__int64 mPausedTime;
	__int64 mStopTime;
	__int64 mPrevTime;
	__int64 mCurrTime;

	bool mStopped;
};


