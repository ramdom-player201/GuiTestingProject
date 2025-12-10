#include <string>

enum class LogType {
	CRITICAL,	// immediate fault that may lead to crash or data corruption, may lead to a forced exit									:: [Dark Red]
	ERROR,		// like assertion, should never be encountered, indicative of a problem in program that may have have undefined results :: [Red]
	ABNORM,		// shouldn't crash, but something is outside of standard operational parameters											:: [Purple]
	WIP,		// missing/incomplete feature, should include todo message in log														:: [Magenta]

	HIGH,		// something important			:: [Orange]
	MED,		// something lower priority		:: [Yellow]
	LOW,		// something low priority		:: [White]

	TRACE,		// tracks the chain of operations of the program between activities														:: [Cyan]
	SPAM,		// rapidly called and not stored																						:: [Grey]
	FILTER,		// when a SPAM log meets certain parameters, a FILTER copy should be kept so it doesn't get expired by other SPAM logs	:: [Green]
};

class LogService {
private:

public:
	static void Initialise();	// setup LogService, create LogService folder if needed
	static void Flush();		// flush cached logs to file (rate limited)

	static void Log(LogType logType, std::string source, std::string function, std::string message);
	// logType	<- how important the log is
	// source	<- which class created the log
	// function	<- which function created the log
	// message  <- what the log should say
};