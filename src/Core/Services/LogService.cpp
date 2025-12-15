#include "LogService.h"
#include <iostream>

LogQueue<20> LogService::Logs_CRITICAL;
LogQueue<250> LogService::Logs_ERROR;
LogQueue<100> LogService::Logs_ABNORM;
LogQueue<100> LogService::Logs_WIP;
LogQueue<100> LogService::Logs_HIGH;
LogQueue<100> LogService::Logs_MED;
LogQueue<100> LogService::Logs_LOW;
LogQueue<1000> LogService::Logs_TRACE;
LogQueue<100> LogService::Logs_SPAM;
LogQueue<100> LogService::Logs_CATCH;

void LogService::Initialise()
{
	std::cout << "LogService::Initialise()" << std::endl;
	std::cout << sizeof(Logs_TRACE) << std::endl;
	std::cout << sizeof(LogType) << std::endl;
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

void LogService::Log(const LogType logType,const std::string& source, const std::string& function, const std::string& message)
{
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
