#ifndef DBQUERY_H
#define DBQUERY_H

#include <string>
#include <string.h>
#include <map>
#include "xsockets/xsocketinterface.h"
using namespace std;

struct Blob
{
	Blob()
	{
		blobData = NULL;
		blobLen = 0;
	}
	Blob(char * _blobData, unsigned int _blobLen)
	{
		blobData = (char *) malloc(_blobLen);
		memcpy(blobData, _blobData, _blobLen);
		blobLen = _blobLen;
	}
	void destroy()
	{
		if (blobData)
			free(blobData);
	}
	char * blobData;
	unsigned int blobLen;
};

class DBQuery
{
public:

	DBQuery();
	virtual ~DBQuery();
	DBQuery(const DBQuery & dbq);
	DBQuery(const string & _query);

	char * GetQuery();
	unsigned int GetQueryLen();
	map<unsigned int, Blob> * GetBlobs();
	void AddBlob(unsigned int position, char * blobData, unsigned int blobLen);

	bool SocketSerialize(XSocketInterface * s) const;
	bool SocketUnSerialize(XSocketInterface * s);

protected:
	string query;
private:
	map<unsigned int, Blob> blobs;

};

#endif // DBQUERY_H
