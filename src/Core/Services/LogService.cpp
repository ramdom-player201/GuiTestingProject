#include "LogService.h"
#include <iostream>

LogService::LogQueue<20> LogService::Logs_CRITICAL;
LogService::LogQueue<250> LogService::Logs_ERROR;
LogService::LogQueue<100> LogService::Logs_ABNORM;
LogService::LogQueue<100> LogService::Logs_WIP;
LogService::LogQueue<20> LogService::Logs_SECURITY;
LogService::LogQueue<100> LogService::Logs_HIGH;
LogService::LogQueue<100> LogService::Logs_MED;
LogService::LogQueue<100> LogService::Logs_LOW;
LogService::LogQueue<1000> LogService::Logs_TRACE;
LogService::LogQueue<100> LogService::Logs_SPAM;
LogService::LogQueue<100> LogService::Logs_CATCH;

void LogService::Initialise()
{
	std::cout << "LogService::Initialise()" << std::endl;
}

void LogService::Flush()
{
}

//struct LogEntry { <- data structure reference only
//LogType type;
//std::chrono::system_clock::time_point timestamp;
//std::string source;
//std::string function;
//std::string message;
//};

void LogService::Log(const LogType logType, const std::string& source, const std::string& function, const std::string& message)
{
	std::cout << "Temp -> LogService :: Log :: {source=[ " + source + " ]function=[ " + function + " ]message=[ " + message + " ]}\n";

	LogEntry newEntry{
		logType,
		std::chrono::system_clock::now(),
		source,
		function,
		message
	};

	switch (logType) {
		//-----------------//
	case LogType::CRITICAL: {
		Logs_CRITICAL.pushLog(newEntry);
		break;
	}
	case LogType::ERROR: {
		Logs_ERROR.pushLog(newEntry);
		break;
	}
	case LogType::ABNORM: {
		Logs_ABNORM.pushLog(newEntry);
		break;
	}
	case LogType::WIP: {
		Logs_WIP.pushLog(newEntry);
		break;
	}
	case LogType::SECURITY: {
		Logs_SECURITY.pushLog(newEntry);
		break;
	}
					 //-----------------//
	case LogType::HIGH: {
		Logs_HIGH.pushLog(newEntry);
		break;
	}
	case LogType::MED: {
		Logs_MED.pushLog(newEntry);
		break;
	}
	case LogType::LOW: {
		Logs_LOW.pushLog(newEntry);
		break;
	}
					 //-----------------//
	case LogType::TRACE: {
		Logs_TRACE.pushLog(newEntry);
		break;
	}
	case LogType::SPAM: {
		Logs_SPAM.pushLog(newEntry);
		break;
	}
	case LogType::CATCH: {
		Logs_CATCH.pushLog(newEntry);
		break;
	}
					   //-----------------//
	default: {
		Logs_ERROR.pushLog(
			{
				LogType::ERROR,
				std::chrono::system_clock::now(),
				"LogService",
				"Log",
				"Error: Log failed because invalid LogType :: Source = " + source + " :: Function = " + function
			}
		);
	}
	}
}

//size_t LogService::GetCurrentDataUsage()
//{
//	// get size of each log, then add together
//	size_t totalSize{ 0 };
//
//	return size_t();
//}
