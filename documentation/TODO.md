# Fix window closing, allow WindowManager to remove closed windows from vector without messing up id system.							[X]
# Consider circumstances surrounding direct window access and potential issues with current system.										[ ]
# Consider transitioning to a map such that windows can maintain same id without vector shifting.										[X]

# Add proper command processing to CommandConsole																						[ ]

# Investigate ansi colours rendering incorrectly when running binary exe directly, but rendering correctly when ran via VisualStudio.	[ ]

# Sort of Object tree with child/parent architecture. Allow windows to contain objects via ObjectManager.								[ ]
# Evaluate whether to make all services into Objects.																					[X]
Services will be static, not objects

# Test that Services are not directly instanciatable and that Services system functions correctly.										[X]
Can remove custom singleton logic and just use static

# Evaluate and implement a new input system.																							[ ]

# Allow windows to report when in focus and allow WindowManager to select a single active window to route inputs into.					[ ]
# Or would the window use InputManager to get inputs when InFocus, ignoring WindowManager completely?									[ ]

# Implement InputManager and input abstraction layer.																					[ ]

# Implement pages																														[ ]

# Modify CommandConsole to bring console to foreground																					[ ]

# Implement debug logging																												[ ]
Write debug logs to file, print number of debugs at program termination
Come up with a structure to define how and when debug logs are cleaned, and ensure memory limits
Mark logs with categories for sorting: by class, by function, etc
Mark logs with types, eg: CRITICAL, HIGH, MED, TRACE, SPAM

CRITICAL	<- immediate fault that may lead to crash or data corruption
ERROR		<- like assertion, should never be encountered, indicative of a problem in program that may have have undefined results
ABNORM		<- shouldn't crash, but something is outside of standard operational parameters
WIP			<- missing/incomplete feature, should include todo message in log

HIGH		<- something important
MED			<- something lower priority
LOW			<- something low priority

TRACE		<- tracks the chain of operations of the program between activities
SPAM		<- rapidly called and not stored
FILTER		<- when a SPAM log meets certain parameters, a FILTER copy should be kept so it doesn't get expired by other SPAM logs