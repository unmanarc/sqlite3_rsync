#include "sqlitedbconnector.h"
#include <unistd.h>
#include <vector>
#include "global_args.h"
#include "helpers/helper_string.h"

extern globalArgs_t globalArgs;

#define PRINT_ON_VERBOSE(x) if(globalArgs.verbosity>0) fprintf(stdout,"%s",x)
/*
 static int callback(void *, int argc, char **argv, char **azColName)
 {
 int i;
 for(i=0; i<argc; i++)
 {
 printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
 }
 printf("\n");
 return 0;
 }*/

SQLiteDBConnector::SQLiteDBConnector()
{
	ppDb = NULL;
}

SQLiteDBConnector::~SQLiteDBConnector()
{
	if (ppDb)
		sqlite3_close(ppDb);
}

void SQLiteDBConnector::StartDB(const std::string &_dbFile, bool readOnly)
{
	dbFile = _dbFile;

	if (access(dbFile.c_str(), R_OK))
	{
		if (readOnly)
		{
			fprintf(stderr, "Can't open database: File does not exist\n");
			_exit(0);
		}
		printf("Database %s does not exist. creating database...\n",
				dbFile.c_str());
	}

	int rc;
	if (!readOnly)
		rc = sqlite3_open(dbFile.c_str(), &ppDb);
	else
		rc = sqlite3_open_v2(dbFile.c_str(), &ppDb, SQLITE_OPEN_READONLY, NULL);

	if (rc)
	{
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(ppDb));
		_exit(0);
	}

	// db opened.
	printf("Database Ready.\n");
}

bool SQLiteDBConnector::ExecQuery(DBQuery *query)
{
	const char *tail;
	map<unsigned int, Blob> * blobs = query->GetBlobs();
	sqlite3_stmt *stmt = 0;
	sqlite3_prepare_v2(ppDb, query->GetQuery(), query->GetQueryLen() + 1, &stmt,
			&tail);
	for (auto blob : *blobs)
	{
		sqlite3_bind_blob(stmt, blob.first, blob.second.blobData,
				blob.second.blobLen, SQLITE_STATIC);
	}
	if (sqlite3_step(stmt) != SQLITE_DONE)
	{
		sqlite3_finalize(stmt);
		fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ppDb));
		return false;
	}
	sqlite3_reset(stmt);
	sqlite3_clear_bindings(stmt);
	sqlite3_finalize(stmt);
	PRINT_ON_VERBOSE("[V] -> Query executed successfully.\n");
	return true;
}
/*
 bool SQLiteDBConnector::ExecQueries(const std::list<std::string> &queries)
 {
 char *zErrMsg = 0;
 int rc,i=1;
 for (auto query : queries)
 {
 rc = sqlite3_exec(ppDb, query.c_str(), callback, 0, &zErrMsg);
 if( rc != SQLITE_OK )
 {
 fprintf(stderr, "SQL error: %s\n", zErrMsg);
 sqlite3_free(zErrMsg);
 return false;
 }else
 {
 PRINT_ON_VERBOSE("[V] -> Query executed successfully.\n");
 }
 i++;
 }
 return true;
 }
 */
bool SQLiteDBConnector::CheckIfTableExist(const std::string &table)
{
	bool ret;
	string xsql = "select sql from sqlite_master where tbl_name=\"?\";";
	sqlite3_stmt * stmt;
	sqlite3_prepare_v2(ppDb, xsql.c_str(), xsql.size() + 1, &stmt, NULL);
	sqlite3_bind_text(stmt, 1, /* The number of the argument. */
	table.c_str(), table.size(), 0 /* The callback. */
	);
	int s = sqlite3_step(stmt);
	ret = (s == SQLITE_ROW ? true : false);
	sqlite3_reset(stmt);
	sqlite3_clear_bindings(stmt);
	sqlite3_finalize(stmt);
	return ret;
}

std::string SQLiteDBConnector::GetCreateTable(const std::string &table)
{
	std::string rsp;
	string xsql =
			"SELECT sql FROM (SELECT sql sql, type type, tbl_name tbl_name, name name, rowid x FROM sqlite_master UNION ALL SELECT sql, type, tbl_name, name, rowid FROM sqlite_temp_master) WHERE lower(tbl_name) LIKE '?' AND type!='meta' AND sql NOTNULL ORDER BY rowid";
	sqlite3_stmt * stmt;
	sqlite3_prepare_v2(ppDb, xsql.c_str(), xsql.size() + 1, &stmt, NULL);
	sqlite3_bind_text(stmt, 1, /* The number of the argument. */
	table.c_str(), table.size(), 0 /* The callback. */
	);
	while (1)
	{
		int s;
		s = sqlite3_step(stmt);
		if (s == SQLITE_ROW)
		{
			const unsigned char * text;
			text = sqlite3_column_text(stmt, 0);
			rsp += (string) (char *) text;
			rsp += (string) ";\n";
		}
		else if (s == SQLITE_DONE)
		{
			break;
		}
		else
		{
			fprintf(stderr, "Get Table Schema Failed.\n");
			exit(1);
		}
	}
	sqlite3_reset(stmt);
	sqlite3_clear_bindings(stmt);
	sqlite3_finalize(stmt);

	return rsp;
}

std::list<std::string> SQLiteDBConnector::GetOIDSForTable(
		const std::string &table)
{
	std::list<std::string> r;

	string xsql = "SELECT oid AS TEXT FROM ?;";

	sqlite3_stmt * stmt;
	sqlite3_prepare_v2(ppDb, xsql.c_str(), xsql.size() + 1, &stmt, NULL);
	sqlite3_bind_text(stmt, 1, /* The number of the argument. */
	table.c_str(), table.size(), 0 /* The callback. */
	);
	while (1)
	{
		int s;
		s = sqlite3_step(stmt);
		if (s == SQLITE_ROW)
		{
			const unsigned char * text;
			text = sqlite3_column_text(stmt, 0);
			r.push_back((string) (char *) text);
		}
		else if (s == SQLITE_DONE)
		{
			break;
		}
		else
		{
			fprintf(stderr, "Get OIDS failed.\n");
			exit(1);
		}
	}
	sqlite3_reset(stmt);
	sqlite3_clear_bindings(stmt);
	sqlite3_finalize(stmt);

	return r;
}

std::list<DBQuery *> SQLiteDBConnector::GetQueriesForOIDS(
		const std::string &tableName, const std::list<u_int64_t> &oids,
		u_int64_t *maxoid)
{
	std::list<DBQuery *> r;
	if (maxoid)
		*maxoid = 0;
	for (u_int64_t oid : oids)
	{
		r.push_back(GetQueryForOID(tableName, oid));
		if (maxoid && oid > *maxoid)
			*maxoid = oid;
	}
	return r;
}

std::list<DBQuery *> SQLiteDBConnector::GetQueriesForOIDSGreaterThan(
		const std::string &tableName, u_int64_t minoid, u_int64_t *lastoid)
{
	return GetQueriesForOIDS(tableName, GetOIDSGreaterThan(tableName, minoid),
			lastoid);
}

std::list<u_int64_t> SQLiteDBConnector::GetOIDSGreaterThan(
		const string &tableName, u_int64_t minoid)
{
	std::list<u_int64_t> r;

	string sMinoid = UI64ToString(minoid);
	string xsql = "SELECT oid AS TEXT FROM ? WHERE oid>?;";

	sqlite3_stmt * stmt;
	sqlite3_prepare_v2(ppDb, xsql.c_str(), xsql.size() + 1, &stmt, NULL);
	sqlite3_bind_text(stmt, 1, /* The number of the argument. */
	tableName.c_str(), tableName.size(), 0 /* The callback. */
	);
	sqlite3_bind_text(stmt, 2, /* The number of the argument. */
	sMinoid.c_str(), sMinoid.size(), 0 /* The callback. */
	);
	while (1)
	{
		int s;
		s = sqlite3_step(stmt);
		if (s == SQLITE_ROW)
		{
			const unsigned char * text;
			text = sqlite3_column_text(stmt, 0);
			r.push_back(StringToUI64((string) (char *) text));
		}
		else if (s == SQLITE_DONE)
		{
			break;
		}
		else
		{
			fprintf(stderr, "Get OIDS failed.\n");
			exit(1);
		}
	}
	sqlite3_reset(stmt);
	sqlite3_clear_bindings(stmt);
	sqlite3_finalize(stmt);
	return r;
}

std::vector<std::string> STLSplit(const string & x, char * d)
{
	std::vector<std::string> v;
	char * x2 = strdup(x.c_str());
	char *saveptr;
	for (char * token = strtok_r(x2, d, &saveptr); token;
			token = strtok_r(NULL, d, &saveptr))
	{
		v.push_back(token);
	}
	free(x2);
	return v;
}

void SQLiteDBConnector::GetTableComponents(const std::string &tableName,
		DBQueryConstructor *dbqc)
{
	dbqc->setTable(tableName);
	dbqc->AddParameterDefinition("oid", "INTEGER");

	string xsql =
			"select sql from sqlite_master where tbl_name='?' and type='table';";

	sqlite3_stmt * stmt;
	sqlite3_prepare_v2(ppDb, xsql.c_str(), xsql.size() + 1, &stmt, NULL);
	sqlite3_bind_text(stmt, 1, /* The number of the argument. */
	tableName.c_str(), tableName.size(), 0 /* The callback. */
	);
	int s;
	s = sqlite3_step(stmt);
	if (s == SQLITE_ROW)
	{
		char * text = strdup((char *) sqlite3_column_text(stmt, 0));
		char * def_start = strchr(text, '(');
		if (def_start != NULL)
		{
			def_start++;
			char * def_end = strchr(def_start, ')');
			def_end[0] = 0;
			vector<string> wparams = STLSplit(def_start, ",");
			for (string singleParam : wparams)
			{
				vector<string> vSingleParam = STLSplit(singleParam, " ");
				if (vSingleParam.size() >= 2)
				{
					string singleParamName = vSingleParam[0];
					string singleParamType = vSingleParam[1];

					dbqc->AddParameterDefinition(singleParamName,
							singleParamType);
				}
			}
		}
		free(text);
	}
	else if (s == SQLITE_DONE)
	{
		// No results... :( null query.
	}
	else
	{
		fprintf(stderr, "Get Table Components failed...\n");
		exit(1);
	}
	sqlite3_reset(stmt);
	sqlite3_clear_bindings(stmt);
	sqlite3_finalize(stmt);
}

DBQuery *SQLiteDBConnector::GetQueryForOID(const std::string &tableName,
		u_int64_t oid)
{
	DBQueryConstructor * dbqc = new DBQueryConstructor;
	GetTableComponents(tableName, dbqc);
	FillInsertQuery(dbqc, oid);
	return dbqc;
}

void SQLiteDBConnector::FillInsertQuery(DBQueryConstructor *dbqc, u_int64_t oid)
{
	DataSelect ds = dbqc->GetDataSelect(oid);

	sqlite3_stmt * stmt;
	sqlite3_prepare_v2(ppDb, ds.query.c_str(), ds.query.size() + 1, &stmt,
			NULL);
	int s;
	s = sqlite3_step(stmt);
	if (s == SQLITE_ROW)
	{
		for (unsigned int i = 0; i < ds.params.size(); i++)
		{
			DataParameter p = ds.params[i];
			if (!p.blob)
			{
				char * text = strdup((char *) sqlite3_column_text(stmt, i));
				dbqc->AddParameterValue(p.paramName, text);
				free(text);
			}
			else
			{
				unsigned int len = sqlite3_column_bytes(stmt, i);
				char * data = (char *) sqlite3_column_blob(stmt, i);
				dbqc->AddParameterAsBLOB(p.paramName, data, len);
			}
		}
	}
	else if (s == SQLITE_DONE)
	{
		// No results... :( null query.
	}
	else
	{
		fprintf(stderr, "Get Table Components failed...\n");
		exit(1);
	}
	sqlite3_reset(stmt);
	sqlite3_clear_bindings(stmt);
	sqlite3_finalize(stmt);
}
