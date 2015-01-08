#include "sqlite3syncclient.h"
#include <unistd.h>
#include "db/dbquery.h"
#include "helpers/helper_nodes.h"
#include "defines.h"
#include "verbose.h"

SQLite3SyncClient::SQLite3SyncClient()
{
	monitorStarted = false;
	lastRetrievedOID = 0;
	sTcpPort = 38742;
	interval = 5;
}

void SQLite3SyncClient::LoadDB(string _dbFile, string _currentTable)
{
	monitorStarted = false;
	dbFile = _dbFile;
	currentTable = _currentTable;

	lastRetrievedOID = 0;

	dbConnector.StartDB(_dbFile, true);

	// db opened.
	printf("Database Reader Ready for table %s.\n", _currentTable.c_str());
}

void SQLite3SyncClient::ProcessDBItems()
{
	AddQueries(dbConnector.GetQueriesForOIDSGreaterThan(currentTable, lastRetrievedOID, &lastRetrievedOID));
	sleep(interval);
}

void * ThreadedRetriveRows(void * data)
{
	SQLite3SyncClient * client = (SQLite3SyncClient *) data;
	while (1)
	{
		client->ProcessDBItems();
	}
	pthread_exit(NULL);
}

void SQLite3SyncClient::StartMonitoringDB()
{
	int r;
	while (true)
	{
		// Connect
		if (!serverConnection.IsConnected())
		{
			while (!serverConnection.Connect(remoteHost, sTcpPort))
			{
				ReconnectTimeout();
				printf("Retrying connection to %s:%d.\n", remoteHost.c_str(), sTcpPort);
			}
		}
		printf("[%p] Connected to %s:%d\n", this, remoteHost.c_str(), sTcpPort);

		XSocketInterface cmdiface = serverConnection;

		PRINT_ON_VERBOSE_2("Table",currentTable.c_str());

		cmdiface.WriteStringLow("SetTable");
		cmdiface.WriteString(currentTable, MAX_TABLENAMESIZE);
		if (!cmdiface.ReadUChar())
		{
			PRINT_ON_VERBOSE("Table does not exist on remote server, creating it...");

			if ((r = ExecQuery(&cmdiface, dbConnector.GetCreateTable(currentTable))) < 0)
			{
				// reconnect.
				continue;
			}
		}

		// Grab UUID.
		cmdiface.WriteStringLow("GetServerUUID");
		string currentUUID = cmdiface.ReadStringLow();

		PRINT_ON_VERBOSE_2("Server UUID",currentUUID.c_str());

		// SYNC (different uuid, or first time)
		if (remoteUUID != currentUUID)
		{
			std::list<std::string> clientNodes = dbConnector.GetOIDSForTable(currentTable);

			// Compress it before send.
			clientNodes = CompressOIDNodes(clientNodes);

			PRINT_ON_VERBOSE_STRING_LIST("New UUID Detected, sending Client OID's table...",clientNodes);

			cmdiface.WriteStringLow("GetMissingNodes");
			cmdiface.WriteStringList(clientNodes, MAX_OIDSIZE);

			bool ok;
			list<string> missingNodes = cmdiface.ReadStringList(&ok,MAX_OIDSIZE);
			if (!ok)
			{
				serverConnection.Close();
				ReconnectTimeout();
				continue;
			}

			PRINT_ON_VERBOSE_STRING_LIST("Receiving server missing nodes...",clientNodes);

			missingNodes = ExpandOIDNodes(missingNodes);
			list<u_int64_t> missingNodes64 = GetOIDNodesByUInt64(missingNodes);
			AddQueries(dbConnector.GetQueriesForOIDS(currentTable, missingNodes64, &lastRetrievedOID));
			remoteUUID = currentUUID; // synced.
		}

		if (!monitorStarted)
		{
			monitorStarted = true;
			pthread_t pt;
			pthread_create(&pt, NULL, ThreadedRetriveRows, this);
		}

		while (1)
		{
			DBQuery* query = (DBQuery*) heapQueries.GetElement();

			if (globalArgs.verbosity>=1)
			{
				printf("[%p] Remote executing query...: [%s]\n",this,query->GetQuery());
			}

			if ((r = ExecQuery(&cmdiface, *query)) == -1)
			{
				// Reinsert and reconnect.
				heapQueries.AddElement(query);
				break;
			}
			else if (r == -2)
				heapQueries.AddElement(query); // Just Reinsert
			else
				delete query; // Continue.
		}
	}
}

unsigned short SQLite3SyncClient::getSTcpPort() const
{
	return sTcpPort;
}

void SQLite3SyncClient::setSTcpPort(unsigned short value)
{
	sTcpPort = value;
}

string SQLite3SyncClient::getCurrentTable() const
{
	return currentTable;
}

void SQLite3SyncClient::setCurrentTable(const string &value)
{
	currentTable = value;
}
string SQLite3SyncClient::getRemoteHost() const
{
	return remoteHost;
}

void SQLite3SyncClient::setRemoteHost(const string &value)
{
	remoteHost = value;
}

void SQLite3SyncClient::ReconnectTimeout()
{
	sleep(5);
}
unsigned int SQLite3SyncClient::getInterval() const
{
	return interval;
}

void SQLite3SyncClient::setInterval(unsigned int value)
{
	interval = value;
}

int SQLite3SyncClient::ExecQuery(XSocketInterface *cmdiface, const string &query)
{
	DBQuery q(query);
	return ExecQuery(cmdiface, q);
}

int SQLite3SyncClient::ExecQuery(XSocketInterface *cmdiface, const DBQuery &query)
{
	cmdiface->WriteStringLow("ExecQuery");

	// Send and execute query.
	query.SocketSerialize(cmdiface);

	bool ok;
	unsigned char rsp = cmdiface->ReadUChar(&ok);

	if (!ok)
	{
		//Connection dropped... // Reinsert...
		serverConnection.Close();
		ReconnectTimeout();
		return -1;
	}
	else
	{
		// Connection OK
		if (!rsp)
		{
			// Reinsert stop and pass....
			printf("[%p] Insertion Error (@query)... Reinserting at (timeout)...\n",this);
			ReconnectTimeout();
			return -2;
		}
		else
		{
			// Processed OK!
			return 0;
		}
	}
}

void SQLite3SyncClient::AddQueries(std::list<DBQuery *> queries)
{
	for (DBQuery * query : queries)
	{
		heapQueries.AddElement(query);
	}
}
