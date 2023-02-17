#pragma once

class Timer
{
public:
	Timer()
	{
		__int64 countsPerSec{};
		QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
		m_SecondsPerCount = 1.0 / (double)countsPerSec;
	}

	float TotalTime()
	{
		if (bIsStopped)
		{
			return static_cast<float>(((m_StopTime - m_PausedTime) - m_BaseTime) * m_SecondsPerCount);
		}

		return static_cast<float>(((m_CurrentTime - m_PausedTime) - m_BaseTime) * m_SecondsPerCount);
	}

	float DeltaTime()
	{
		return static_cast<float>(m_DeltaTime);
	}

	void Reset()
	{
		__int64 currentTime{};
		QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);

		m_BaseTime = currentTime;
		m_PreviousTime = currentTime;
		m_StopTime = 0;
		bIsStopped = false;
	}

	void Start()
	{
		__int64 startTime{};
		QueryPerformanceCounter((LARGE_INTEGER*)&startTime);

		if (bIsStopped)
		{
			m_PausedTime += (startTime - m_StopTime);

			m_PreviousTime = startTime;
			m_StopTime = 0;
			bIsStopped = false;
		}
	}

	void Stop()
	{
		if (!bIsStopped)
		{
			__int64 currentTime{};
			QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);

			m_StopTime = currentTime;
			bIsStopped = true;
		}
	}

	void Tick()
	{
		if (bIsStopped)
		{
			m_DeltaTime = 0.0;
			return;
		}

		__int64 currentTime{};
		QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);
		m_CurrentTime = currentTime;

		m_DeltaTime = (m_CurrentTime - m_PreviousTime) * m_SecondsPerCount;
		m_PreviousTime = m_CurrentTime;

		if (m_DeltaTime < 0.0)
		{
			m_DeltaTime = 0.0f;
		}

	}

	void GetFrameStats()
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
	double m_SecondsPerCount{};
	double m_DeltaTime{};

	int64_t m_BaseTime{};
	int64_t m_PausedTime{};
	int64_t m_StopTime{};
	int64_t m_PreviousTime{};
	int64_t m_CurrentTime{};

	bool bIsStopped{ false };

public:
	int m_FrameCount{};
	float m_TimeElapsed{};
	int m_FPS{};
	float m_Miliseconds{};
};
