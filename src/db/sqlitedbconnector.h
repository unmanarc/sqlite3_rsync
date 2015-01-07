#ifndef SQLITEDBCONNECTOR_H
#define SQLITEDBCONNECTOR_H

#include <list>
#include <string>
#include "dbquery.h"
#include "dbqueryconstructor.h"

#include "sqlite3.h"

struct TableColumn
{
	std::string columnName;
	std::string columnType;
};

typedef std::list<TableColumn> TableComponents;

class SQLiteDBConnector
{
public:
	SQLiteDBConnector();
	~SQLiteDBConnector();

	void StartDB(const std::string & _dbFile, bool readOnly = false);
	bool ExecQuery(DBQuery * query);
	//bool ExecQueries(const std::list<std::string> & queries);
	bool CheckIfTableExist(const std::string &table);
	std::string GetCreateTable(const std::string &table);
	std::list<std::string> GetOIDSForTable(const std::string &table);
	std::list<DBQuery *> GetQueriesForOIDS(const std::string &tableName,
			const std::list<u_int64_t> &oids, u_int64_t * maxoid);
	std::list<DBQuery *> GetQueriesForOIDSGreaterThan(
			const std::string &tableName, u_int64_t minoid,
			u_int64_t * lastoid);

private:
	std::list<u_int64_t> GetOIDSGreaterThan(const std::string &tableName,
			u_int64_t minoid);

	void GetTableComponents(const std::string &tableName,
			DBQueryConstructor * dbqc);
	DBQuery *GetQueryForOID(const std::string &tableName, u_int64_t oid);

	void FillInsertQuery(DBQueryConstructor * dbqc, u_int64_t oid);

	std::string dbFile;
	sqlite3 *ppDb;

};

#endif // SQLITEDBCONNECTOR_H
