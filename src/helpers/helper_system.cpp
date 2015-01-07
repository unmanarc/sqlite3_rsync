#include "helper_system.h"
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <string>

std::string GetFirstLine(std::string path, unsigned int max)
{
	std::string rsp;
	char * x = (char *) malloc(max + 1);
	std::ifstream in(path.c_str());
	if (!in)
	{
		free(x);
		in.close();
		return rsp;
	}
	in.getline(x, max);
	in.close();
	rsp = x;
	free(x);
	return rsp;
}

std::string GetUUID()
{
	return GetFirstLine("/proc/sys/kernel/random/uuid", 64);
}
