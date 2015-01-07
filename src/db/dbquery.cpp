#include "dbquery.h"

DBQuery::DBQuery()
{

}

DBQuery::~DBQuery()
{
	for (auto blob : blobs)
	{
		Blob b = blob.second;
		b.destroy();
	}
}

DBQuery::DBQuery(const string &_query)
{
	query = _query;
}

char *DBQuery::GetQuery()
{
	return (char *) query.c_str();
}

unsigned int DBQuery::GetQueryLen()
{
	return query.size();
}

map<unsigned int, Blob> *DBQuery::GetBlobs()
{
	return &blobs;
}

void DBQuery::AddBlob(unsigned int position, char *blobData,
		unsigned int blobLen)
{
	Blob b(blobData, blobLen);
	blobs[position] = b;
}

bool DBQuery::SocketSerialize(XSocketInterface *s) const
{
	s->WriteString(query);
	for (auto blob : blobs)
	{
		s->WriteUChar(1);
		s->WriteUInt32(blob.first);
		s->WriteBlock(blob.second.blobData, blob.second.blobLen);
	}
	s->WriteUChar(0);
	return true;
}

bool DBQuery::SocketUnSerialize(XSocketInterface *s)
{
	query = s->ReadString();
	while (s->ReadUChar())
	{
		unsigned int pos = s->ReadUInt32(), len;
		char * data = (char *) s->ReadBlockAlloc(&len);
		Blob b;
		b.blobData = data;
		b.blobLen = len;
		blobs[pos] = b;
	}
	return true;
}

DBQuery::DBQuery(const DBQuery &dbq)
{
	// Copy the query!.
	query = dbq.query;
	// Copy the map:
	for (auto blob : blobs)
	{
		blobs[blob.first] = blob.second;
	}
}

