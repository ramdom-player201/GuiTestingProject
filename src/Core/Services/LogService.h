#pragma once

#include <string>
#include <chrono>
#include <array>

//#include "../ConsoleColours.h" // allow anything using console to use colours ERROR: cannot use because ConsoleColours uses logs

enum class LogType : uint8_t {
	CRITICAL,	// immediate fault that may lead to crash or data corruption, may lead to a forced exit									:: [Dark Red]
	ERROR,		// like assertion, should never be encountered, indicative of a problem in program that may have have undefined results :: [Red]
	ABNORM,		// shouldn't crash, but something is outside of standard operational parameters											:: [Purple]
	WIP,		// missing/incomplete feature, should include todo message in log														:: [Magenta]
	SECURITY,	// program attempts to do something not allowed, such as access a file outside of its directory							:: [Blue]

	HIGH,		// something important			:: [Orange]
	MED,		// something lower priority		:: [Yellow]
	LOW,		// something low priority		:: [White]

	USER,		// trace user inputs, such as clicking a button, etc																	:: [Orange]
	TEST,		// temporarily test a feature
	SUCCESS,	// something occured successfully
	FAIL,		// something failed, but is not unrecoverable (not an error)

	TRACE,		// tracks the chain of operations of the program between activities														:: [Cyan]
	SPAM,		// rapidly called and not stored																						:: [Grey]
	CATCH,		// when a SPAM log meets certain parameters, a CATCH copy should be kept so it doesn't get expired by other SPAM logs	:: [Green]
};

struct LogEntry {
	LogType type{ LogType::ERROR };
	std::chrono::system_clock::time_point timestamp;
	std::string source;
	std::string function;
	std::string message;
	uint16_t copies{ 0 };
};

class LogService {
private:

	// LogQueue nested class begin
	template <size_t queueSize>
	class LogQueue {
	private:
		std::array<LogEntry, queueSize> queue; // collection of logs
		size_t head; // position of head in circular queue
		size_t fillCount; // counts up until one full cycle
		size_t dropped; // number of logs overwritten
	public:
		void pushLog(const LogEntry& entry) {
			queue[head] = entry; // add entry to queue at current head position
			head = (head + 1) % queueSize;
			if (fillCount < queueSize) {
				fillCount++;
			}
			else {
				dropped++;
			}
		}
		size_t getQueueSize() { return sizeof(queue); } // return size of queue array, in bytes
		size_t getDropCount() { return dropped; } // return number of expired logs

		constexpr LogQueue() : head(0), fillCount(0), dropped(0) {} // initialisation of variables must be done here, because class is nested
	};
	// LogQueue nested class end


	// list of category circular queues
	static inline LogQueue<20> Logs_CRITICAL;
	static inline LogQueue<250> Logs_ERROR;
	static inline LogQueue<100> Logs_ABNORM;
	static inline LogQueue<100> Logs_WIP;
	static inline LogQueue<20> Logs_SECURITY;

	static inline LogQueue<100> Logs_HIGH;
	static inline LogQueue<100> Logs_MED;
	static inline LogQueue<100> Logs_LOW;

	static inline LogQueue<20> Logs_USER;
	static inline LogQueue<100> Logs_TEST;
	static inline LogQueue<100> Logs_SUCCESS;
	static inline LogQueue<100> Logs_FAIL;

	static inline LogQueue<1000> Logs_TRACE;
	static inline LogQueue<100> Logs_SPAM;
	static inline LogQueue<100> Logs_CATCH;

	// Used for separating logs in output
	static inline std::string previousSource;

public:
	static void Initialise();	// setup LogService, create LogService folder if needed
	static void Flush();		// flush cached logs to file (rate limited)

	static void Separator();

	static void Log(const LogType logType, const std::string_view source, const std::string_view function, const std::string_view message);

	static size_t GetCurrentDataUsage();

	// logType	<- how important the log is
	// source	<- which class created the log
	// function	<- which function created the log
	// message  <- what the log should say

	/* TODO:
		Log needs to store logs in fixed-size circular queues
		Each logType needs to store logs in a separate queue
		Consider size-profile of log structures, how many logs is reasonable to store and how much storage would it use?
		How long should each log file persist for? (days)

		Consider what file structure will be used to store log files
		Ensure initialise generates file structure if missing
		Ensure initialise clears old logs
		Add FileManager service for managing files and folders

		{Directory}/ProjectFiles/DebugLogs/{date}/{logType}.log

		Windows are owned by WindowManager, LogService does not have its own dedicated window
		LogService gui is stored in a UiPanel that can be given to a Window.
		UiPanel is shared, so multiple windows can have the same tile.

		LogService gui functions:
		* live output
		* browse/manage log files
		* view log file contents
		* STARTUP_NOTIFICATION: view critical logs from last session
	*/
};