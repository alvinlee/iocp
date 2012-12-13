#include "StdAfx.h"
#include ".\samplequeue.h"

SampleQueue::SampleQueue(void)
{
	InitializeCriticalSection(&cs_);
	SetParam(0,0,0);
}

SampleQueue::~SampleQueue(void)
{
	DeleteCriticalSection(&cs_);
}

void SampleQueue::AddSample( DWORD bytes )
{
	EnterCriticalSection(&cs_);

	if(sampleList_.empty())AddNewSample(bytes);
	else{
		CheckSampleByCount();
		CheckSampleByTime();

		BOOL goNext = FALSE;
		SAMPLE &lastSample = sampleList_.back();
		// overwrite ?
		if( sampleOverwriteTime_ > 0 && 
			GetTickCount() - lastSample.beginTime > sampleOverwriteTime_)
			goNext = TRUE;

		// overflow ? associate with max count
		if( sampleMaxCount_ > 0 &&
			lastSample.bytes * sampleMaxCount_ + bytes < lastSample.bytes)
			goNext = TRUE;

		if(goNext)AddNewSample(bytes);
		else lastSample.bytes += bytes;
	}

	LeaveCriticalSection(&cs_);
}

void SampleQueue::GetInfo(DWORD &rate,DWORD &during,ULONGLONG &bytes)
{
	EnterCriticalSection(&cs_);

	CheckSampleByCount();
	CheckSampleByTime();

	rate = 0;
	during = 0;
	bytes = 0;

	if(sampleList_.empty()){
		LeaveCriticalSection(&cs_);
		return;
	}

	DWORD beginTime = 0;

	for (SAMPLELIST::iterator it = sampleList_.begin();
		it != sampleList_.end();++it)
	{
		if(beginTime > (*it).beginTime || beginTime == 0){
			beginTime = (*it).beginTime;
		}

		bytes += (*it).bytes;
	}

	during = GetTickCount() - beginTime;
	during /= 1000;
	bytes /= 1000;
	if(during)rate = bytes/during;

	LeaveCriticalSection(&cs_);
}

void SampleQueue::Reset()
{
	EnterCriticalSection(&cs_);
	sampleList_.clear();
	LeaveCriticalSection(&cs_);
}

void SampleQueue::SetParam( DWORD lifeTime,DWORD overwriteTime,DWORD maxCount )
{
	EnterCriticalSection(&cs_);
	sampleLifeTime_ = lifeTime;
	sampleOverwriteTime_ = overwriteTime;
	sampleMaxCount_ = maxCount;
	LeaveCriticalSection(&cs_);
}

void SampleQueue::AddNewSample( DWORD bytes )
{
	SAMPLE sample = {GetTickCount(),bytes};
	sampleList_.push_back(sample);
}

void SampleQueue::CheckSampleByCount()
{
	if(sampleMaxCount_ == 0)return;
	while(sampleList_.size() > sampleMaxCount_)sampleList_.pop_front();
}


void SampleQueue::CheckSampleByTime()
{
	if(sampleLifeTime_ == 0)return;

	if(sampleList_.empty())return;

	DWORD deadLine = GetTickCount() - sampleLifeTime_;

	while (!sampleList_.empty() && sampleList_.front().beginTime < deadLine)
		sampleList_.pop_front();
}