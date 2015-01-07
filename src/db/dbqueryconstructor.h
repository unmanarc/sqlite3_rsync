#ifndef DBQUERYCONSTRUCTOR_H
#define DBQUERYCONSTRUCTOR_H

#include "dbquery.h"
#include <string>
#include <map>
#include <vector>

using namespace std;

struct DataParameter
{
	string paramName;
	bool blob;
};

struct DataSelect
{
	string query;
	vector<DataParameter> params;
};

class DBQueryConstructor: public DBQuery
{
public:
	DBQueryConstructor();

	string getTable() const;
	void setTable(const string &value);

	void AddParameterDefinition(const string & paramName, const string & paramType);
	void AddParameterValue(const string & paramName, const string & paramValue);
	void AddParameterAsBLOB(const string & paramName, char * data, unsigned int len);
	void ConstructQuery();

	DataSelect GetDataSelect(unsigned long long oid);

	~DBQueryConstructor();
private:

	string SQLFilter(const string & sqlValue);

	string table;
	map<string, string> paramDefs;
	map<string, string> paramValues;
	map<unsigned int, string> paramBlobs;
	unsigned int blobCount;
};

#endif // DBQUERYCONSTRUCTOR_H
