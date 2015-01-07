#ifndef GLOBAL_ARGS_H
#define GLOBAL_ARGS_H

#include <set>
#include <string>

using namespace std;

struct globalArgs_t
{
	int verbosity; /* -v option */
	int asServer;
	char * database;
	set<string> tables;
	int interval;
	unsigned int port;
	char * remoteHost;
};

#endif // GLOBAL_ARGS_H
