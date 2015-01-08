#include "xsocketinterface.h"
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

XSocketInterface::XSocketInterface()
{
}

bool XSocketInterface::WriteUChar(const unsigned char &c)
{
	unsigned char snd[1];
	snd[0] = c;
	return Write(&snd, 1);
}

unsigned char XSocketInterface::ReadUChar(bool * bok)
{
	unsigned char rsp[1] =
	{ 0 };
	if (bok)
		*bok = true;
	if (!Read(&rsp, 1) && bok)
		*bok = false;
	return rsp[0];
}

unsigned int XSocketInterface::ReadUInt32(bool *bok)
{
	unsigned int rsp = 0;
	if (bok)
		*bok = true;
	if (!Read(&rsp, sizeof(unsigned int)) && bok)
		*bok = false;
	return htonl(rsp);
}

bool XSocketInterface::WriteUInt32(const unsigned int &c)
{
	unsigned int c1 = htonl(c);
	return Write(&c1, sizeof(unsigned int));
}

int XSocketInterface::ReadInt32(bool *bok)
{
	int rsp = 0;
	if (bok)
		*bok = true;
	if (!Read(&rsp, sizeof(int)) && bok)
		*bok = false;
	return rsp;
}

bool XSocketInterface::WriteInt32(const int &c)
{
	return Write(&c, sizeof(int));
}

bool XSocketInterface::WriteBlock(const void *data, uint32_t datalen)
{
	datalen = htonl(datalen);
	if (!Write(&datalen, sizeof(uint32_t)))
		return false;
	datalen = ntohl(datalen);
	return (Write(data, datalen));
}

bool XSocketInterface::ReadBlock(void *data, uint32_t datalen, bool bestrict)
{
	uint32_t downSize = 0;
	if (!Read(&downSize, sizeof(uint32_t)))
		return false;

	downSize = ntohl(downSize);

	// Being strict with the block size. // avoid errors with data structures.
	if (bestrict && datalen != downSize)
		return false;

	if (downSize > datalen)
	{
		// download and resize
		unsigned char * odata = new unsigned char[downSize];
		bool ok = Read(odata, downSize);
		if (ok)
			memcpy(data, odata, datalen);
		delete[] odata;
		if (!ok)
			return false;
	}
	else
	{
		// Go straight forward with the data array.
		memset(data, 0, datalen);
		if (!Read(data, downSize))
			return false;
	}
	return true;
}

void *XSocketInterface::ReadBlockAlloc(unsigned int *datalen)
{
	*datalen = 0;
	uint32_t downSize = 0;
	if (!Read(&downSize, sizeof(uint32_t)))
		return NULL;

	void * rbl = malloc(downSize);
	downSize = ntohl(downSize);

	memset(rbl, 0, downSize);
	if (!Read(rbl, downSize))
	{
		free(rbl);
		return NULL;
	}

	*datalen = downSize;

	return rbl;
}

list<string> XSocketInterface::ReadStringList(bool *bok, unsigned int max)
{
	list<string> rsl;
	unsigned int elements = ReadUInt32(bok);

	for (unsigned int i = 0; i < elements; i++)
	{
		string dat = ReadString(max);
		rsl.push_front(dat);
	}

	return rsl;
}

bool XSocketInterface::WriteStringList(const list<string> &val, unsigned int max)
{
	if (!WriteUInt32(val.size()))
		return false;
	for (auto v : val)
	{
		if (!WriteString(v, max))
			return false;
	}
	return true;
}

string XSocketInterface::ReadStringLow()
{
	string rsp;
	u_char downSize = 0;

	if (!Read(&downSize, sizeof(u_char)) || !downSize)
		return "";

	// generar odata
	char * odata = new char[downSize + 2];

	// configurar odata como NUL
	memset(odata, 0, downSize + 2);

	// leer data.
	bool ok = Read(odata, downSize);
	if (ok)
		rsp = odata;

	// Eliminar datos raw.
	delete[] odata;

	// responder data recibida.
	return rsp;
}

bool XSocketInterface::WriteStringLow(const string &val)
{
	// Establecer el valor de envio 0-255
	u_char upSize = (val.length() > 255 ? 255 : val.length());
	// Enviar
	if (!Write(&upSize, sizeof(u_char)))
		return false;
	// Enviar datos.
	return (Write(val.c_str(), upSize));
}

string XSocketInterface::ReadString(unsigned int max, bool *okx)
{
	if (okx)
		*okx = true;
	string rsp;
	unsigned int downSize = 0;
	if (!Read(&downSize, sizeof(unsigned int)) || !downSize)
	{
		if (okx)
			*okx = false;
		return "";
	}
	downSize = ntohl(downSize);
	if (downSize > max)
	{
		if (okx)
			*okx = false;
		fprintf(stderr, "Read String Overflow... Closing socket\n");
		fflush(stderr);
		Close();
		return "";
	}
	// generar odata
	char * odata = new char[downSize + 2];
	// configurar odata como NUL
	memset(odata, 0, downSize + 2);
	// leer data.
	bool ok = Read(odata, downSize);
	if (ok)
		rsp = odata;
	else
	{
		if (okx)
			*okx = false;
	}
	// Eliminar datos raw.
	delete[] odata;
	// responder data recibida.
	return rsp;
}

bool XSocketInterface::WriteString(const string &val, unsigned int max)
{
	// Establecer el valor de envio 0-255
	unsigned int upSize = (val.length() > max ? max : val.length());
	upSize = htonl(upSize);
	// Enviar
	if (!Write(&upSize, sizeof(unsigned int)))
		return false;

	// Enviar datos.
	return (Write(val.c_str(), ntohl(upSize)));
}
