#include "Logger.hpp"
#include <spdlog/sinks/msvc_sink.h>

#include <string>

#if defined (_DEBUG) || (DEBUG)
	std::string Logger::m_LoggerName = "Debug";
#else
	std::string Logger::m_LoggerName = "Release";
#endif

Logger::~Logger()
{
	m_Logger.reset();
	m_Logger = nullptr;
}

void Logger::Init()
{
	auto msvcSink{ std::make_shared<spdlog::sinks::msvc_sink_mt>() };
	m_Logger = std::make_shared<spdlog::logger>(m_LoggerName, msvcSink);
	spdlog::register_logger(m_Logger);
	
#if defined (_DEBUG) || (DEBUG)
	spdlog::set_level(spdlog::level::debug);
#else
	spdlog::set_level(spdlog::level::info);
#endif

}

void Logger::Log(const std::string& Message, LogType Type)
{
	switch (Type)
	{
	case LogType::eInfo:
		spdlog::get(Logger::m_LoggerName)->info(Message.data());
		break;
	case LogType::eWarning:
		spdlog::get(Logger::m_LoggerName)->warn(Message.data());
		break;
	case LogType::eError:
		spdlog::get(Logger::m_LoggerName)->error(Message.data());
		break;
	case LogType::eCritical:
		spdlog::get(Logger::m_LoggerName)->critical(Message.data());
		break;
	default:
		spdlog::get(Logger::m_LoggerName)->warn("Invalid Log Type used!");
		break;
	}
}
