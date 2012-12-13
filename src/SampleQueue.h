#pragma once

class SampleQueue
{
public:
	typedef struct {
		DWORD beginTime;
		ULONGLONG bytes;
	} SAMPLE;
	typedef std::list<SAMPLE> SAMPLELIST;

	SampleQueue(void);
	~SampleQueue(void);

	void AddSample(DWORD bytes);
	void GetInfo(DWORD &rate,DWORD &during,ULONGLONG &bytes); 
	void Reset();
	void SetParam(DWORD lifeTime,DWORD overwriteTime,DWORD maxCount); // 0:ignore

protected:
	void AddNewSample(DWORD bytes);
	void CheckSampleByCount();
	void CheckSampleByTime();

	SAMPLELIST sampleList_;
	CRITICAL_SECTION cs_;

	DWORD sampleLifeTime_;
	DWORD sampleOverwriteTime_;
	size_t sampleMaxCount_;
};
