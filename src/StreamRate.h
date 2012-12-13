#pragma once

#include "SampleQueue.h"

class StreamRate
{
public:
	StreamRate(void);
	~StreamRate(void);
	static StreamRate Total;

	void Enable(BOOL enable);
	void Reset();

	void AddInBytes(DWORD inBytes);
	void AddOutBytes(DWORD outBytes);

	// kbytes per second
	DWORD GetDuring(); 
	ULONGLONG GetInBytes(); 
	ULONGLONG GetOutBytes(); 

	DWORD GetAvgInRate(); 
	DWORD GetAvgOutRate(); 
	DWORD GetCurInRate(); 
	DWORD GetCurOutRate(); 

private:
	static const size_t SAMPLE_COUNT = 30;
	static const DWORD SAMPLE_DURING = 3000;
	static const DWORD SAMPLE_OVERWRITE_TIME = 100;

	volatile BOOL enable_;

	SampleQueue avgInSample_;
	SampleQueue avgOutSample_;
	SampleQueue curInSample_;
	SampleQueue curOutSample_;
};
