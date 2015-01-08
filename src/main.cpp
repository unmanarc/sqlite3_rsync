/*
 ============================================================================
 Name        : sqlite3_rsync.cpp
 Author      : Aarón Mizrachi
 Version     :
 Copyright   : Unmanarc (C) 2015 - LGPL
 Description : sqlite3 rsync in C++,
 ============================================================================
 */
#include <iostream>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string>
#include <list>

#include "global_args.h"
#include "sqlite3syncclient.h"
#include "sqlite3syncserver.h"

#include "helpers/helper_system.h"

string appuuid;

globalArgs_t globalArgs;
using namespace std;

void help_and_exit()
{
	cout << "Help:" << endl;
	cout << "-----" << endl;
	cout << "sqlite3_rsync synchronizes SQLite3 databases from the client to the server using tables OID." << endl;
	cout << endl;
	cout << "Server Mode:" << endl;
	cout << "------------" << endl;
	cout << "-s --server <port>       : Act as TCP server" << endl;
	cout << "-d --database <file>     : Local database filepath" << endl;
	cout << endl;
	cout << "Client Mode:" << endl;
	cout << "------------" << endl;
	cout << "-c --client <host:port>  : Act as TCP client" << endl;
	cout << "-d --database <file>     : Local database filepath" << endl;
	cout << "-t --tables <t1,t2,...>  : Comma separated list of tables to be synchronized" << endl;
	cout << "-i --interval <seconds>  : Seconds between each upgrade attempt" << endl;
	cout << endl;
	cout << "Other Options:" << endl;
	cout << "--------------" << endl;
	cout << "-v --verbose             : Be Verbose" << endl;
	cout << "-h --help                : Show help" << endl;
	_exit(0);
}

struct SQLite3SyncClientForTableArgs
{
	string table;
};

// For each table, there is a client thread connected to the master.
void * SQLite3SyncClientForTableThread(void * data)
{
	SQLite3SyncClientForTableArgs * args = (SQLite3SyncClientForTableArgs *) data;

	SQLite3SyncClient client;
	client.setRemoteHost(globalArgs.remoteHost);
	client.setSTcpPort(globalArgs.port);
	client.LoadDB(globalArgs.database, args->table);
	client.setInterval(globalArgs.interval);

	while (1)
	{
		client.StartMonitoringDB();
		sleep(globalArgs.interval);
	}

	delete args;
	pthread_exit(NULL);
	return NULL;
}

int main(int argc, char *argv[])
{
	cout << "SQLite3 R-Sync " << PACKAGE_VERSION << " - Done by Unmanarc - <aaron@unmanarc.com>" << endl;

	int opt = 0, longIndex;

	appuuid = GetUUID();

	/* Initialize globalArgs before we get to work. */
	globalArgs.asServer = 1;
	globalArgs.verbosity = 0;
	globalArgs.database = NULL;
	globalArgs.interval = 5;
	globalArgs.port = 38742;
	globalArgs.remoteHost = NULL;

	static const char *optString = "s::c:d:t:i:vh?";

	static const struct option longOpts[] =
	{
	{ "interval", required_argument, NULL, 'i' },
	{ "tables", required_argument, NULL, 't' },
	{ "database", required_argument, NULL, 'd' },
	{ "client", required_argument, NULL, 'c' },
	{ "server", optional_argument, NULL, 's' },
	{ "verbose", no_argument, NULL, 'v' },
	{ "help", no_argument, NULL, 'h' },
	{ NULL, no_argument, NULL, 0 } };

	opt = getopt_long(argc, argv, optString, longOpts, &longIndex);
	while (opt != -1)
	{
		switch (opt)
		{
		case 'i':
			if (optarg && *optarg && atoi(optarg))
			{
				globalArgs.interval = atoi(optarg);
			}
			break;
		case 't':
		{
			char * tables = strdup(optarg);
			const char * delim = ",";
			char *save, *p;
			for (p = strtok_r(tables, delim, &save); p; p = strtok_r(NULL, delim, &save))
			{
				string tableName = p;
				globalArgs.tables.insert(tableName);
			}
			free(tables);
		}
			break;
		case 'd':
		{
			globalArgs.database = optarg;
		}
			break;
		case 'c':
		{
			globalArgs.asServer = 0;

			globalArgs.remoteHost = strdup(optarg);
			char * remotePort = strchr(globalArgs.remoteHost, ':');

			if (remotePort)
			{
				remotePort[0] = 0;
				remotePort++;
				if (*remotePort && atoi(remotePort))
				{
					globalArgs.port = atoi(remotePort);
				}
			}
		}
			break;
		case 's':
		{
			globalArgs.asServer = 1;

			if (optarg && *optarg && atoi(optarg))
			{
				globalArgs.port = atoi(optarg);
			}
		}
			break;
		case 'v':
			globalArgs.verbosity++;
			break;
		case 'h': /* fall-through is intentional */
		case '?':
			help_and_exit();
			break;
		case 0: /* long option without a short arg */
			/*if( strcmp( "randomize", longOpts[longIndex].name ) == 0 ) {
			 globalArgs.randomized = 1;
			 }*/
			break;

		default:
			/* You won't actually get here. */
			break;
		}
		opt = getopt_long(argc, argv, optString, longOpts, &longIndex);
	}

	bool terminate = false;
	if (!globalArgs.database)
	{
		fprintf(stderr, "Error: please specify database filename (-d filename)\n");
		terminate = true;
	}
	if (globalArgs.tables.empty() && !globalArgs.asServer)
	{
		fprintf(stderr, "Error: please specify tables to sync (-t table1,table2,...)\n");
		terminate = true;
	}
	if (!globalArgs.asServer && !globalArgs.remoteHost)
	{
		fprintf(stderr, "Error: please specify the remote host (-c host:port)\n");
		terminate = true;
	}
	if (access(globalArgs.database, R_OK) && !globalArgs.asServer)
	{
		fprintf(stderr, "Database not found.\n");
		terminate = true;
	}
	if (terminate)
		exit(1);

	if (globalArgs.asServer)
	{
		SQLite3SyncServer server;
		server.setSTcpPort(globalArgs.port);
		server.LoadDB(globalArgs.database);
		server.StartListenerThreads();
	}
	else
	{
		list<pthread_t *> pts;

		for (string table : globalArgs.tables)
		{
			SQLite3SyncClientForTableArgs * arg = new SQLite3SyncClientForTableArgs;
			arg->table = table;

			pthread_t * pt = new pthread_t;
			pthread_create(pt, NULL, SQLite3SyncClientForTableThread, arg);
			pts.push_front(pt);
		}

		for (pthread_t * pt : pts)
		{
			pthread_join(*pt, NULL);
		}
	}
	return 0;
}

