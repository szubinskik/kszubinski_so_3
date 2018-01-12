#include "debug.h"
#include <ctime>

VERBOSITY _v;

const char  *YELLOW = "\033[1;33m",
			*CYAN = "\033[0;36m",
			*RED = "\033[0;31m",
			*NC = "\033[0m";

void debug_init(VERBOSITY v)
{
	_v = v;
}

bool should_log()
{
	if (_v >= V_LOG)
		return true;

	return false;
}

bool should_error()
{
	if (_v >= V_ERROR)
		return true;

	return false;
}

bool should_trace()
{
	if (_v >= V_TRACE)
		return true;

	return false;
}

void LOG(std::string msg)
{
	if (should_log())
		std::cout << "[" << CYAN << "LOG" << NC << "]" << "\t" << "[" <<  std::time(0) << "] " << msg << std::endl;
}

void C_LOG(const char* msg)
{
	if (should_log())
		printf("[%sLOG%s]\t[%ld] %s\n", CYAN, NC, std::time(0), msg);
}

void ERROR(std::string msg)
{
	if (should_error())
		std::cout << "[" << RED << "ERROR" << NC << "]" << "\t" << "[" <<  std::time(0) << "] " << msg << std::endl;
}

void C_ERROR(const char* msg)
{
	if (should_error())
		printf("[%sERROR%s]\t[%ld] %s\n", RED, NC, std::time(0), msg);
}

void TRACE(std::string msg)
{
	if (should_trace())
		std::cout << "[" << YELLOW << "TRACE" << NC << "]" << "\t" << "[" <<  std::time(0) << "] " << msg << std::endl;
}

void C_TRACE(const char* msg)
{
	if (should_trace())
		printf("[%sTRACE%s]\t[%ld] %s\n", YELLOW, NC, std::time(0), msg);
}