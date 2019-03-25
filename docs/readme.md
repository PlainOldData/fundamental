# Tyrant Job Readme

Fiber based job system.

Overview
- A job can be dependent on other jobs completing without holding up the thread.
- An active fiber doesn't migrate cores (other impls do).

## Example

_See unit tests or functional test_

## Credits

Thanks to Naughtdog's Christian Gyrling GDC Talk
https://www.gdcvault.com/play/1022186/Parallelizing-the-Naughty-Dog-Engine

And RichieSams open source implimentation
https://github.com/RichieSams/FiberTaskingLib
