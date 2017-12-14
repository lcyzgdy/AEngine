#pragma once
#ifndef __DTIMER_H__
#define __DTIMER_H__

#include"onwind.h"
using namespace std;

namespace AEngine
{
#ifdef WIN32
	class DTimer : public NonCopyable
	{
		LARGE_INTEGER qpcFrequency;
		LARGE_INTEGER qpcLastTime;
		uint64_t qpcMaxDelta;
		// 时间数据源使用QPC单元

		uint64_t elapsedTicks;
		uint64_t totalTicks;
		uint64_t leftOverTicks;	// ???

		uint64_t qpcSecondCounter;
		uint32_t FrameCount;
		uint32_t framesPerSecond;
		uint32_t framesThisSecond;

		bool isFixedTimeStep;
		uint64_t targetElapsedTicks;
		// 用于配置固定帧率模式

	public:
		DTimer();
		~DTimer();

		const uint64_t GetElapsedTicks();
		const double GetElapsedSeconds();
		// 获取自上次调用以来所经过的时间

		const uint64_t GetTotalTicks();
		const double GetTotalSeconds();
		// 获取程序启动后的时间

		uint32_t GetFrameCount();
		// 获取程序启动以来更新的帧数

		uint32_t GetFramePerSecond();
		// 获取帧率

		void SetFixedFramerate(bool _isFixedTimeStep);
		// 设置是否使用固定帧率

		void SetTargetElapsedTicks(uint64_t _targetElapsed);
		void SetTargetElapsedSeconds(double _targetElapsed);
		// 设置更新频率

		static const uint64_t TicksPerSecond = 1000000;
		// 设置每秒使用1000000个时钟周期

		static double TicksToSeconds(uint64_t _ticks);
		static UINT64 SecondsToTicks(double _seconds);

		void ResetElapsedTime();
		// 在长时间中断（如阻塞IO、线程中断）后调用以追赶更新进度

		typedef void(*LpUpdateFunc) (void);

		void Tick(LpUpdateFunc _update);
		// 更新计时器状态，调用指定次数的update函数（OnUpdate）
	};

	using StepTimer = DTimer;
#endif
}
#endif // !__DTIMER_H__
