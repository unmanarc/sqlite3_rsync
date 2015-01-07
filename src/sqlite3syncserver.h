#ifndef SQLITE3SYNCSERVER_H
#define SQLITE3SYNCSERVER_H

#include <string>
#include <list>
#include "xsockets/xsocket.h"
#include "db/sqlitedbconnector.h"

#include "global_args.h"

extern globalArgs_t globalArgs;

class SQLite3SyncServer
{
public:
    SQLite3SyncServer();
    ~SQLite3SyncServer();

    void AttendClient(XSocket sock);

    void LoadDB(string _dbFile);
    void StartListenerThreads();

    unsigned short getSTcpPort() const;
    void setSTcpPort(unsigned short value);
private:
    unsigned short sTcpPort;
    SQLiteDBConnector dbConnector;
};

#endif // SQLITE3SYNCSERVER_H
