#include "dbqueryconstructor.h"
#include "helpers/helper_string.h"


DBQueryConstructor::DBQueryConstructor() :
		DBQuery()
{
	blobCount = 1;
}

DBQueryConstructor::~DBQueryConstructor()
{

}

string DBQueryConstructor::SQLFilter(const string &sqlValue)
{
	string r = sqlValue;
	find_and_replace(r, "\\", "\\\\");
	find_and_replace(r, "'", "\\'");
	find_and_replace(r, "\"", "\\\"");
	return r;
}
string DBQueryConstructor::getTable() const
{
	return table;
}

void DBQueryConstructor::setTable(const string &value)
{
	table = value;
}

void DBQueryConstructor::AddParameterDefinition(const string &paramName, const string &paramType)
{
	paramDefs[paramName] = paramType;
}

void DBQueryConstructor::AddParameterValue(const string &paramName, const string &paramValue)
{
	paramValues[paramName] = paramValue;
}

void DBQueryConstructor::AddParameterAsBLOB(const string &paramName, char *data, unsigned int len)
{
	AddBlob(blobCount, data, len);
	paramBlobs[blobCount++] = paramName;
}

void DBQueryConstructor::ConstructQuery()
{
	string xquery_params, xquery_values;
	bool first = true;
	for (auto param : paramValues)
	{
		if (first)
		{
			first = false;
		}
		else
		{
			xquery_params += ",";
			xquery_values += ",";
		}
		xquery_params += "`" + param.first + "`";
		xquery_values += "'" + SQLFilter(param.second) + "'";
	}
	for (unsigned int i = 1; i < blobCount; i++)
	{
		if (first)
		{
			first = false;
		}
		else
		{
			xquery_params += ",";
			xquery_values += ",";
		}
		xquery_params += string("`") + paramBlobs[i] + string("`");
		xquery_values += "?";
	}
	query = "INSERT INTO `" + table + "` (" + xquery_params + ") values(" + xquery_values + ");\n";
}

DataSelect DBQueryConstructor::GetDataSelect(unsigned long long oid)
{
	DataSelect r;
	r.query = "SELECT oid";
	DataParameter p;

	p.paramName = "rowid";
	p.blob = false;
	r.params.push_back(p);

	for (auto param : paramDefs)
	{
		p.paramName = param.first;

		if (p.paramName!="oid")
		{
			string paramType = param.second;

			p.blob = (paramType == "BLOB");

			r.query += "," + p.paramName;
			r.params.push_back(p);
		}
	}

	r.query += " FROM " + table + string(" WHERE oid='") + UI64ToString(oid) + string("';");
	return r;
}

