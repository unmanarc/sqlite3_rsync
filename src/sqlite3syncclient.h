#ifndef SQLITE3SYNCCLIENT_H
#define SQLITE3SYNCCLIENT_H

#include <string>

#include <stdint.h>

#include "db/sqlitedbconnector.h"
#include "global_args.h"
#include "sqlite3.h"

#include "xsockets/xsocket_tcp.h"
#include "ts/tsheap.h"
#include "xsockets/xsocketinterface.h"

using namespace std;

class SQLite3SyncClient
{
public:
    SQLite3SyncClient();

    void LoadDB(string _dbFile, string _currentTable);

    void ProcessDBItems();

    void StartMonitoringDB();

    unsigned short getSTcpPort() const;
    void setSTcpPort(unsigned short value);

    string getCurrentTable() const;
    void setCurrentTable(const string &value);

    string getRemoteHost() const;
    void setRemoteHost(const string &value);

    unsigned int getInterval() const;
    void setInterval(unsigned int value);

private:
    int ExecQuery(XSocketInterface * cmdiface, const string & query);
    int ExecQuery(XSocketInterface * cmdiface, const DBQuery & query);
    void AddQueries(std::list<DBQuery *> queries);


    TSHeap heapQueries;

    void ReconnectTimeout();

    string remoteUUID;
    bool monitorStarted;

    XTCPSocket serverConnection;

    u_int64_t lastRetrievedOID;
    unsigned int interval;

    SQLiteDBConnector dbConnector;
    unsigned short sTcpPort;
    string remoteHost;
    string currentTable;
    string dbFile;
};

#endif // SQLITE3SYNCCLIENT_H
