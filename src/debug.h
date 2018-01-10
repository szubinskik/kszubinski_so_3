#ifndef DEBUG_H
#define DEBUG_H

#include <iostream>
#include <cstdio>
#include <cstring>
#include <string>

void C_LOG(const char* msg);
void LOG(std::string msg);

void C_ERROR(const char* msg);
void ERROR(std::string msg);

enum VERBOSITY
{
	V_ERROR,
	V_LOG
};

void debug_init(VERBOSITY v);

#endif /* DEBUG_H */