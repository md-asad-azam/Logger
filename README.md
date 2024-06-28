# Logger

*Know issues*

*	This logger is modified to use a queue based model to add the log message in the queue and then log it
	using a separate thread so the the caller thread is never blocked. But there is a problem with this,
	if the caller thread is not blocked and exits before the logger thread then it will alse be able to join 
	before the logging has finished. Hence the Main thread will also not wait for the loggin to finish due to
	which I had to add a sleep of 2s at the end of the main() so that the loggin is completed by that time.

*	Need to fix the formatting of the function and file name getting printed.

*	Could be made more efficient by not using the std::string.