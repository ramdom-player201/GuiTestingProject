#include "LogService.h"

#include <iostream>
#include "../ConsoleColours.h"

//LogService::LogQueue<20> LogService::Logs_CRITICAL;
//LogService::LogQueue<250> LogService::Logs_ERROR;
//LogService::LogQueue<100> LogService::Logs_ABNORM;
//LogService::LogQueue<100> LogService::Logs_WIP;
//LogService::LogQueue<20> LogService::Logs_SECURITY;
//LogService::LogQueue<20> LogService::Logs_USER;
//LogService::LogQueue<100> LogService::Logs_HIGH;
//LogService::LogQueue<100> LogService::Logs_MED;
//LogService::LogQueue<100> LogService::Logs_LOW;
//LogService::LogQueue<100> LogService::Logs_SUCCESS;
//LogService::LogQueue<100> LogService::Logs_TEST;
//LogService::LogQueue<1000> LogService::Logs_TRACE;
//LogService::LogQueue<100> LogService::Logs_SPAM;
//LogService::LogQueue<100> LogService::Logs_CATCH;

void LogService::Initialise()
{
	std::cout << "LogService::Initialise()\n";
	std::cout << "WIP, initialise does nothing right now" << std::endl;
}

void LogService::Flush()
{
	std::cout << "LogService::Flush()\n";
	std::cout << "WIP, flush does nothing right now" << std::endl;
}

void LogService::Separator()
{
	std::cout << std::endl;
}

//struct LogEntry { <- data structure reference only
//LogType type;
//std::chrono::system_clock::time_point timestamp;
//std::string source;
//std::string function;
//std::string message;
//};

struct SpamData {
	bool lastWasSpam{ false }; // debounce spam
	size_t spamCount{ 0 }; // count how many messages were spammed 

	std::string lastSource{ "" };
	std::string lastFunction{ "" };
	std::string lastMessage{ "" };
};
SpamData lastSpamData;

void LogService::Log(const LogType logType, const std::string_view source, const std::string_view function, const std::string_view message)
{
	// Add spaces to separate output from different classes
	if (previousSource != source) {
		std::cout << std::endl;
		previousSource = source;
	}

	// Handle spam
	if (logType == LogType::SPAM) {
		// Spam can be output, but duplicates must be excluded
		lastSpamData.lastWasSpam = true;
		bool outputAndReset{ false };
		if (lastSpamData.lastWasSpam) { // last message was spam, check if duplicate
			if (
				lastSpamData.lastSource == source &&
				lastSpamData.lastFunction == function &&
				lastSpamData.lastMessage == message
				)
			{ // is duplicate, increment counter
				lastSpamData.spamCount++;
			}
			else { // is new, output end state and count
				if (lastSpamData.spamCount > 0) {
					std::cout <<
						ConsoleColours::getColourCode(AnsiColours::DEFAULT) <<
						ConsoleColours::getColourCode(AnsiColours::FLASH) << "Temp -> LogService" <<
						ConsoleColours::getColourCode(AnsiColours::DEFAULT) << " :: {t = [" <<
						ConsoleColours::getColourCode(AnsiColours::CYAN_DARK) << "SPAM" <<
						ConsoleColours::getColourCode(AnsiColours::DEFAULT) << "] <...> [" <<
						ConsoleColours::getColourCode(AnsiColours::CYAN_DARK) << std::to_string(lastSpamData.spamCount) <<
						ConsoleColours::getColourCode(AnsiColours::DEFAULT) << "]\n";
				}
				outputAndReset = true;
			}
		}
		else { // last message was not spam, set data
			outputAndReset = true;
		}

		if (outputAndReset) {
			std::cout <<
				ConsoleColours::getColourCode(AnsiColours::DEFAULT) <<
				ConsoleColours::getColourCode(AnsiColours::FLASH) << "Temp -> LogService" <<
				ConsoleColours::getColourCode(AnsiColours::DEFAULT) << " :: {t = [" <<
				ConsoleColours::getColourCode(AnsiColours::CYAN_DARK) << "SPAM" <<
				ConsoleColours::getColourCode(AnsiColours::DEFAULT) << "]s=[" <<
				ConsoleColours::getColourCode(AnsiColours::ORANGE_DARK) << source <<
				ConsoleColours::getColourCode(AnsiColours::DEFAULT) << "]f=[" <<
				ConsoleColours::getColourCode(AnsiColours::YELLOW_DARK) << function <<
				ConsoleColours::getColourCode(AnsiColours::DEFAULT) << "]m=[" <<
				ConsoleColours::getColourCode(AnsiColours::GREY_MEDIUM_BRIGHT) << message <<
				ConsoleColours::getColourCode(AnsiColours::DEFAULT) << "]}\n";
			lastSpamData.spamCount = 0;
			lastSpamData.lastSource = source;
			lastSpamData.lastFunction = function;
			lastSpamData.lastMessage = message;
		}
	}
	// Handle everything not spam
	else {
		// Output log to default console
		if (lastSpamData.lastWasSpam && lastSpamData.spamCount > 0) {
			std::cout <<
				ConsoleColours::getColourCode(AnsiColours::DEFAULT) <<
				ConsoleColours::getColourCode(AnsiColours::FLASH) << "Temp -> LogService" <<
				ConsoleColours::getColourCode(AnsiColours::DEFAULT) << " :: {t = [" <<
				ConsoleColours::getColourCode(AnsiColours::CYAN_DARK) << "SPAM" <<
				ConsoleColours::getColourCode(AnsiColours::DEFAULT) << "] <...> [" <<
				ConsoleColours::getColourCode(AnsiColours::CYAN_DARK) << std::to_string(lastSpamData.spamCount) <<
				ConsoleColours::getColourCode(AnsiColours::DEFAULT) << "]\n";
		}
		lastSpamData.lastWasSpam = false;
		{
			std::cout <<
				ConsoleColours::getColourCode(AnsiColours::DEFAULT) <<
				ConsoleColours::getColourCode(AnsiColours::FLASH) << "Temp -> LogService" <<
				ConsoleColours::getColourCode(AnsiColours::DEFAULT) << " :: {t = [";

			switch (logType) {
			case LogType::CRITICAL:	/**/ { std::cout << ConsoleColours::getColourCode(AnsiColours::MAGENTA_BRIGHT)		/**/ << "CRITICAL"; /**/ break; }
			case LogType::ERROR:	/**/ { std::cout << ConsoleColours::getColourCode(AnsiColours::RED)					/**/ << "ERROR";	/**/ break; }
			case LogType::ABNORM:	/**/ { std::cout << ConsoleColours::getColourCode(AnsiColours::RED)					/**/ << "ABNORM";	/**/ break; }
			case LogType::WIP:		/**/ { std::cout << ConsoleColours::getColourCode(AnsiColours::FLASH) << ConsoleColours::getColourCode(AnsiColours::PINK)			/**/ << "WIP";	/**/ break; }
			case LogType::SECURITY:	/**/ { std::cout << ConsoleColours::getColourCode(AnsiColours::BLUE_BRIGHT)			/**/ << "SECURITY";	/**/ break; }

			case LogType::HIGH:		/**/ { std::cout << ConsoleColours::getColourCode(AnsiColours::RED_BRIGHT)			/**/ << "HIGH";		/**/ break; }
			case LogType::MED:		/**/ { std::cout << ConsoleColours::getColourCode(AnsiColours::ORANGE_BRIGHT)		/**/ << "MEDIUM";	/**/ break; }
			case LogType::LOW:		/**/ { std::cout << ConsoleColours::getColourCode(AnsiColours::YELLOW_BRIGHT)		/**/ << "LOW";		/**/ break; }

			case LogType::USER:		/**/ { std::cout << ConsoleColours::getColourCode(AnsiColours::FLASH) << ConsoleColours::getColourCode(AnsiColours::GREEN_BRIGHT)	/**/ << "USER";	/**/ break; }
			case LogType::TEST:		/**/ { std::cout << ConsoleColours::getColourCode(AnsiColours::FLASH) << ConsoleColours::getColourCode(AnsiColours::ORANGE_BRIGHT)	/**/ << "TEST";	/**/ break; }
			case LogType::SUCCESS:	/**/ { std::cout << ConsoleColours::getColourCode(AnsiColours::GREEN)				/**/ << "SUCCESS";	/**/ break; }
			case LogType::FAIL:		/**/ { std::cout << ConsoleColours::getColourCode(AnsiColours::RED)					/**/ << "FAIL";		/**/ break; }

			case LogType::TRACE:	/**/ { std::cout << ConsoleColours::getColourCode(AnsiColours::GREY_MEDIUM_DARK)	/**/ << "TRACE";	/**/ break; }
			case LogType::SPAM:		/**/ { std::cout << ConsoleColours::getColourCode(AnsiColours::CYAN_DARK)			/**/ << "SPAM";		/**/ break; }
			case LogType::CATCH:	/**/ { std::cout << ConsoleColours::getColourCode(AnsiColours::CYAN_BRIGHT)			/**/ << "CATCH";	/**/ break; }
			default: { std::cout << ConsoleColours::getColourCode(AnsiColours::RED) << "UNKNOWN"; }
			}

			std::cout <<
				ConsoleColours::getColourCode(AnsiColours::DEFAULT) << "]s=[" <<
				ConsoleColours::getColourCode(AnsiColours::ORANGE_DARK) << source <<
				ConsoleColours::getColourCode(AnsiColours::DEFAULT) << "]f=[" <<
				ConsoleColours::getColourCode(AnsiColours::YELLOW_DARK) << function <<
				ConsoleColours::getColourCode(AnsiColours::DEFAULT) << "]m=[" <<
				ConsoleColours::getColourCode(AnsiColours::GREY_MEDIUM_BRIGHT) << message <<
				ConsoleColours::getColourCode(AnsiColours::DEFAULT) << "]}\n";
		}
	}

	// create new entry
	LogEntry newEntry{
		logType,
		std::chrono::system_clock::now(),
		std::string(source),
		std::string(function),
		std::string(message),
		1,
	};

	// add entry to relevant queue
	switch (logType) {
		//                   //-----------------//
	case LogType::CRITICAL: { Logs_CRITICAL.pushLog(newEntry); break; }
	case LogType::ERROR: { Logs_ERROR.pushLog(newEntry); break; }
	case LogType::ABNORM: { Logs_ABNORM.pushLog(newEntry); break; }
	case LogType::WIP: { Logs_WIP.pushLog(newEntry); break; }
	case LogType::SECURITY: { Logs_SECURITY.pushLog(newEntry); break; }
						  // //-----------------//
	case LogType::HIGH: { Logs_HIGH.pushLog(newEntry); break; }
	case LogType::MED: { Logs_MED.pushLog(newEntry); break; }
	case LogType::LOW: { Logs_LOW.pushLog(newEntry); break; }
					 //	     //-----------------//
	case LogType::USER: { Logs_USER.pushLog(newEntry); break; }
	case LogType::TEST: { Logs_TEST.pushLog(newEntry); break; }
	case LogType::SUCCESS: { Logs_SUCCESS.pushLog(newEntry); break; }
	case LogType::FAIL: { Logs_SUCCESS.pushLog(newEntry); break; }
					  //     //-----------------//
	case LogType::TRACE: { Logs_TRACE.pushLog(newEntry); break; }
	case LogType::SPAM: { Logs_SPAM.pushLog(newEntry); break; }
	case LogType::CATCH: { Logs_CATCH.pushLog(newEntry);	break; }
					   //    //-----------------//
	default:
		Logs_ERROR.pushLog(
			{
				LogType::ERROR,
				std::chrono::system_clock::now(),
				"LogService",
				"Log",
				"Error: Log failed because invalid LogType :: Source = " + std::string(source) + " :: Function = " + std::string(function)
			}
		);
	}
}

size_t LogService::GetCurrentDataUsage()
{
	// get size of each log, then add together
	size_t totalSize{ 0 };

	totalSize += Logs_CRITICAL.getQueueSize();
	totalSize += Logs_ERROR.getQueueSize();
	totalSize += Logs_ABNORM.getQueueSize();
	totalSize += Logs_WIP.getQueueSize();
	totalSize += Logs_SECURITY.getQueueSize();

	totalSize += Logs_HIGH.getQueueSize();
	totalSize += Logs_MED.getQueueSize();
	totalSize += Logs_LOW.getQueueSize();

	totalSize += Logs_USER.getQueueSize();
	totalSize += Logs_TEST.getQueueSize();
	totalSize += Logs_SUCCESS.getQueueSize();
	totalSize += Logs_FAIL.getQueueSize();

	totalSize += Logs_TRACE.getQueueSize();
	totalSize += Logs_SPAM.getQueueSize();
	totalSize += Logs_CATCH.getQueueSize();

	return totalSize;
}