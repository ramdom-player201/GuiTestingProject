Fix window closing, allow WindowManager to remove closed windows from vector without messing up id system.
Consider circumstances surrounding direct window access and potential issues with current system.
Consider transitioning to a map such that windows can maintain same id without vector shifting.

Add proper command processing to CommandConsole

Investigate ansi colours rendering incorrectly when running binary exe directly, but rendering correctly when ran via VisualStudio.

Sort of Object tree with child/parent architecture. Allow windows to contain objects via ObjectManager.
Evaluate whether to make all services into Objects.

Test that Services are not directly instanciatable and that Services system functions correctly.

Evaluate and implement a new input system.

Allow windows to report when in focus and allow WindowManager to select a single active window to route inputs into.
Or would the window use InputManager to get inputs when InFocus, ignoring WindowManager completely?

Implement InputManager and input abstraction layer.