#include "sqlite3syncserver.h"
#include <pthread.h>
#include <stdio.h>
#include "db/dbquery.h"
#include "xsockets/xsocket_tcp.h"
#include "xsockets/xsocketinterface.h"
#include <unistd.h>
#include "verbose.h"
#include "defines.h"
#include "helpers/helper_nodes.h"

extern string appuuid;

struct ClientThreadParms
{
	SQLite3SyncServer * server;
	XSocket sock;
};

void * ClientThread(void * data)
{
	ClientThreadParms * ctp = (ClientThreadParms *) data;
	string remotePair = ctp->sock.getRemotePair();

	if (globalArgs.verbosity > 0)
	{
		fprintf(stdout, "[%p] Handling new connection from %s (@%p)\n", ctp, remotePair.c_str(),ctp->server);
	}

	ctp->server->AttendClient(ctp->sock);
	delete ctp;
	pthread_exit(NULL);
	return NULL;
}

SQLite3SyncServer::SQLite3SyncServer()
{
	sTcpPort = 38742;
}

SQLite3SyncServer::~SQLite3SyncServer()
{
}

void SQLite3SyncServer::AttendClient(XSocket sock)
{
	XSocketInterface cmdiface = sock;
	string currentTable;
	string cmd;

	while (1)
	{
		cmd = cmdiface.ReadStringLow();
		if (cmd != "")
		{
			if (cmd == "SetTable")
			{
				currentTable = cmdiface.ReadString(MAX_TABLENAMESIZE);
				PRINT_ON_VERBOSE_2("Set Table", currentTable.c_str());
				cmdiface.WriteUChar(dbConnector.CheckIfTableExist(currentTable) ? 1 : 0);
			}
			else if (cmd == "GetServerUUID")
			{
				cmdiface.WriteStringLow(appuuid);
			}
			else if (cmd == "GetMissingNodes")
			{
				bool ok;
				// Read remote nodes.
				list<string> clientNodes = cmdiface.ReadStringList(&ok,MAX_OIDSIZE);

				PRINT_ON_VERBOSE_STRING_LIST("GetMissingNodes (client nodes)",clientNodes);

				if (!ok)
				{
					CONNECTION_FAILURE_RET(sock, "Network error, closing connection\n");
				}

				// Expand nodes (uncompress)
				clientNodes = ExpandOIDNodes(clientNodes);

				// Calc missing nodes.
				list<string> currentNodes = dbConnector.GetOIDSForTable(currentTable);
				list<string> missingNodes = CalcMissingNodes(clientNodes, currentNodes);

				// Compress missing nodes (for transmission)
				missingNodes = CompressOIDNodes(missingNodes);

				PRINT_ON_VERBOSE_STRING_LIST("GetMissingNodes (left nodes)",missingNodes);

				// Send missing nodes.
				if (!cmdiface.WriteStringList(missingNodes, MAX_OIDSIZE))
					CONNECTION_FAILURE_RET(sock, "Network error, closing connection\n");
			}
			else if (cmd == "ExecQuery")
			{
				// Execute queries on the database here.
				DBQuery q;
				q.SocketUnSerialize(&cmdiface);
				PRINT_ON_VERBOSE_2("SQL Query",q.GetQuery());
				cmdiface.WriteUChar(dbConnector.ExecQuery(&q)?1:0);
			}
			else if (cmd == "Exit")
			{
				// Proper exit.
				PRINT_ON_VERBOSE("[V] Closing connection (by remote peer)\n");
				sock.Close();
				return;
			}
		}
		else CONNECTION_FAILURE_RET(sock, "[V] Network error, closing connection\n");
	}
}

void SQLite3SyncServer::LoadDB(string _dbFile)
{
	dbConnector.StartDB(_dbFile);
}

void SQLite3SyncServer::StartListenerThreads()
{
	XTCPSocket tcpSocket;
	if (!tcpSocket.Listen(sTcpPort, "0.0.0.0", true))
	{
		fprintf(stderr, "Can't listen on port %d\n", sTcpPort);
		_exit(0);
	}
	XSocket curSocket;
	do
	{
		curSocket = tcpSocket.Accept();
		if (curSocket.GetSocket() != 0)
		{
			ClientThreadParms * ctp = new ClientThreadParms;
			ctp->sock = curSocket;
			ctp->server = this;

			pthread_t x;
			pthread_create(&x, NULL, ClientThread, ctp);
		}
	} while (curSocket.GetSocket() != 0);
}

unsigned short SQLite3SyncServer::getSTcpPort() const
{
	return sTcpPort;
}

void SQLite3SyncServer::setSTcpPort(unsigned short value)
{
	sTcpPort = value;
}
