# FastTouch
Use this library to detect whether your choice of Arduino pin is being touched.
The technique works with any Arduino pin that has an internal pull up resistor that 
can be enabled. It returns a measure of how long it takes for a pin to rise
to a HIGH level after the pull up is enabled. This will be slowed by any
capacitive load added.
