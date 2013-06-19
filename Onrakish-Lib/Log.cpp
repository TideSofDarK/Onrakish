#include "Log.h"

Log::Log(void)
{
}

std::string Log::log(std::string message)
{
	time_t timer;
	char buffer[25];
	struct tm* tm_info;

	time(&timer);
	tm_info = localtime(&timer);

	strftime(buffer, 25, "%Y:%m:%d%H:%M:%S", tm_info);

	std::string temp = (const char*) buffer;
	return "[" + temp + "] " + message;
}

