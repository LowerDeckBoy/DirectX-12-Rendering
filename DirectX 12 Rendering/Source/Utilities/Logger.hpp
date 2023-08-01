#pragma once
#include <spdlog/spdlog.h>
#include <spdlog/sinks/msvc_sink.h>


enum class LogType : uint8_t
{
	eInfo = 0,
	eWarning,
	eError,
	eCritical
};

class Logger
{
private:
	std::shared_ptr<spdlog::logger> m_Logger;

public:
	static std::string m_LoggerName;
	void Init();

	static void Log(const std::string& Message, LogType Type = LogType::eInfo);

};

