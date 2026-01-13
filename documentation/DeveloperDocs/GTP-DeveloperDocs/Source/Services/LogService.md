LogService is responsible for handling debug logs.

```cpp
struct LogEntry {
	LogType type{ LogType::ERROR };
	std::chrono::system_clock::time_point timestamp;
	std::string source;
	std::string function;
	std::string message;
	uint16_t copies;
};
```

LogEntry units have the following fields:
⨽ type		<- defines what type of log this is (eg error, wip note, trace, etc)
⨽ timestamp	<- the time at which the first copy of this log was sent
⨽ source	<- the class this log is from
⨽ function	<- the function in the class this log is from
⨽ message	<- the debug message itself
⨽ copies	<- if this message is sent multiple times in a row, the original will have its copies incremented WIP: not implemented

Logs are written to individual streams depending on their LogType
Each stream is of limited size, so logs will end up being forgotten once they go out of scope

LogService uses or will use:
[[WindowManager]] For creating a DedicatedWindow to view and manage logs
[[FileManager]] For reading/writing log files
[[ConsoleColours]] For writing coloured text to console, temporary until GUI is done

TODO:

Make logs write to file
Make GUI to view/manage logs
Detach logs from console once the GUI viewer is working

Note, developer logs vs user logs?

There are a range of possible optimisations that could be done in the future:
Save frequently used strings to set<string> and store log using number (className and functionName), consider how many possible variants might exist for int size
Consider high speed vs high memory efficiency vs implementation complexity: is it really worth it?