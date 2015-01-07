#include "helper_string.h"
#include <stdio.h>
#include <stdlib.h>

u_int64_t StringToUI64(const std::string &var)
{
	return strtoull(var.c_str(), NULL, 10);
}

std::string UI64ToString(const u_int64_t &var)
{
	char number[128];
	sprintf(number, "%llu", (long long unsigned int) var);
	return (std::string) number;
}

using std::string;

void find_and_replace(string& source, string const& find, string const& replace)
{
	for (std::string::size_type i = 0; (i = source.find(find, i)) != std::string::npos;)
	{
		source.replace(i, find.length(), replace);
		i += replace.length() - find.length() + 1;
	}
}
