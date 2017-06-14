#pragma once
#ifndef __DTIMER_H__
#define __DTIMER_H__

#include"onwind.h"
using namespace std;

class DTimer
{
	LARGE_INTEGER qpcFrequency;
	LARGE_INTEGER qpcLastTime;
	UINT64 qpcMaxDelta;
	// ʱ������Դʹ��QPC��Ԫ

	UINT64 elapsedTicks;
	UINT64 totalTicks;
	UINT64 leftOverTicks;	// ???

	UINT64 qpcSecondCounter;
	UINT32 FrameCount;
	UINT32 framesPerSecond;
	UINT32 framesThisSecond;

	bool isFixedTimeStep;
	UINT64 targetElapsedTicks;
	// �������ù̶�֡��ģʽ

public:
	DTimer();
	~DTimer();

	const UINT64 GetElapsedTicks();
	const double GetElapsedSeconds();
	// ��ȡ���ϴε���������������ʱ��

	const UINT64 GetTotalTicks();
	const double GetTotalSeconds();
	// ��ȡ�����������ʱ��

	UINT32 GetFrameCount();
	// ��ȡ���������������µ�֡��

	UINT32 GetFramePerSecond();
	// ��ȡ֡��

	void SetFixedFramerate(bool _isFixedTimeStep);
	// �����Ƿ�ʹ�ù̶�֡��

	void SetTargetElapsedTicks(UINT64 _targetElapsed);
	void SetTargetElapsedSeconds(double _targetElapsed);
	// ���ø���Ƶ��

	static const UINT64 TicksPerSecond = 1000000;
	// ����ÿ��ʹ��1000000��ʱ������

	static double TicksToSeconds(UINT64 _ticks);
	static UINT64 SecondsToTicks(double _seconds);

	void ResetElapsedTime();
	// �ڳ�ʱ���жϣ�������IO���߳��жϣ��������׷�ϸ��½���

	typedef void(*LpUpdateFunc) (void);

	void Tick(LpUpdateFunc _update);
	// ���¼�ʱ��״̬������ָ��������update������OnUpdate��
};

#endif // !__DTIMER_H__
