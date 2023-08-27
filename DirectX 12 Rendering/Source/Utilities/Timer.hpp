#pragma once
#include <profileapi.h>
//#include <Windows.h>

class Timer
{
public:
	static void Initialize()
	{
		__int64 countsPerSec{};
		::QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
		m_SecondsPerCount = 1.0 / (double)countsPerSec;
	}

	static float TotalTime()
	{
		if (bIsStopped)
			return static_cast<float>(((m_StopTime - m_PausedTime) - m_BaseTime) * m_SecondsPerCount);

		return static_cast<float>(((m_CurrentTime - m_PausedTime) - m_BaseTime) * m_SecondsPerCount);
	}

	static float DeltaTime()
	{
		return static_cast<float>(m_DeltaTime);
	}

	static void Reset()
	{
		__int64 currentTime{};
		::QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);

		m_BaseTime = currentTime;
		m_PreviousTime = currentTime;
		m_StopTime = 0;
		bIsStopped = false;
	}

	static void Start()
	{
		__int64 startTime{};
		::QueryPerformanceCounter((LARGE_INTEGER*)&startTime);

		if (bIsStopped)
		{
			m_PausedTime += (startTime - m_StopTime);

			m_PreviousTime = startTime;
			m_StopTime = 0;
			bIsStopped = false;
		}
	}

	static void Stop()
	{
		if (!bIsStopped)
		{
			__int64 currentTime{};
			::QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);

			m_StopTime = currentTime;
			bIsStopped = true;
		}
	}

	static void Tick()
	{
		if (bIsStopped)
		{
			m_DeltaTime = 0.0;
			return;
		}

		__int64 currentTime{};
		::QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);
		m_CurrentTime = currentTime;

		m_DeltaTime = (m_CurrentTime - m_PreviousTime) * m_SecondsPerCount;
		m_PreviousTime = m_CurrentTime;

		if (m_DeltaTime < 0.0)
		{
			m_DeltaTime = 0.0f;
		}

	}

	static void GetFrameStats()
	{
		//m_FrameCount = 0;
		//m_TimeElapsed = 0.0f;
		m_FrameCount++;

		if ((TotalTime() - m_TimeElapsed) >= 1.0f)
		{
			m_FPS = m_FrameCount;
			m_Miliseconds = 1000.0f / m_FPS;

			m_FrameCount = 0;
			m_TimeElapsed += 1.0f;
		}
	}


private:
	inline static double m_SecondsPerCount{};
	inline static double m_DeltaTime{};

	inline static int64_t m_BaseTime{};
	inline static int64_t m_PausedTime{};
	inline static int64_t m_StopTime{};
	inline static int64_t m_PreviousTime{};
	inline static int64_t m_CurrentTime{};

	inline static bool bIsStopped{ false };

public:
	inline static uint32_t m_FrameCount{};
	inline static float m_TimeElapsed{};
	inline static uint32_t m_FPS{};
	inline static float m_Miliseconds{};
};
