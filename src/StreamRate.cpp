#include "StdAfx.h"
#include ".\streamrate.h"

StreamRate StreamRate::Total;

StreamRate::StreamRate(void):
enable_(FALSE)
{
	avgInSample_.SetParam(0,0,SAMPLE_COUNT);
	avgOutSample_.SetParam(0,0,SAMPLE_COUNT);
	curInSample_.SetParam(SAMPLE_DURING,SAMPLE_OVERWRITE_TIME,0);
	curOutSample_.SetParam(SAMPLE_DURING,SAMPLE_OVERWRITE_TIME,0);
}

StreamRate::~StreamRate(void)
{
}

void StreamRate::Enable( BOOL enable )
{
	InterlockedExchange((LONG *)&enable_,enable);
}

void StreamRate::Reset()
{
	avgInSample_.Reset();
	avgOutSample_.Reset();
	curInSample_.Reset();
	curOutSample_.Reset();
}

void StreamRate::AddInBytes( DWORD inBytes )
{
	if(!Total.enable_ || inBytes == 0)return;
	if(this != &Total)Total.AddInBytes(inBytes);

	avgInSample_.AddSample(inBytes);
	curInSample_.AddSample(inBytes);
}

void StreamRate::AddOutBytes( DWORD outBytes )
{
	if(!Total.enable_ || outBytes == 0)return;
	if(this != &Total)Total.AddOutBytes(outBytes);

	avgOutSample_.AddSample(outBytes);
	curOutSample_.AddSample(outBytes);
}

DWORD StreamRate::GetDuring()
{
	DWORD rate;
	DWORD duringIn;
	DWORD duringOut;
	ULONGLONG bytes;

	avgInSample_.GetInfo(rate,duringIn,bytes);
	avgOutSample_.GetInfo(rate,duringOut,bytes);

	return duringIn > duringOut ? duringIn : duringOut;
}

ULONGLONG StreamRate::GetInBytes()
{
	DWORD rate;
	DWORD during;
	ULONGLONG bytes;

	avgInSample_.GetInfo(rate,during,bytes);
	return bytes;
}

ULONGLONG StreamRate::GetOutBytes()
{
	DWORD rate;
	DWORD during;
	ULONGLONG bytes;

	avgOutSample_.GetInfo(rate,during,bytes);
	return bytes;
}

DWORD StreamRate::GetAvgInRate()
{
	DWORD rate;
	DWORD during;
	ULONGLONG bytes;

	avgInSample_.GetInfo(rate,during,bytes);

	return rate;
}

DWORD StreamRate::GetAvgOutRate()
{
	DWORD rate;
	DWORD during;
	ULONGLONG bytes;

	avgOutSample_.GetInfo(rate,during,bytes);

	return rate;
}

DWORD StreamRate::GetCurInRate()
{
	DWORD rate;
	DWORD during;
	ULONGLONG bytes;

	curInSample_.GetInfo(rate,during,bytes);

	return rate;
}

DWORD StreamRate::GetCurOutRate()
{
	DWORD rate;
	DWORD during;
	ULONGLONG bytes;

	curOutSample_.GetInfo(rate,during,bytes);

	return rate;
}