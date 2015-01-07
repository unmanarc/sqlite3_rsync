#ifndef XSOCKETINTERFACE_H
#define XSOCKETINTERFACE_H
#include "xsocket.h"

#include <string>
#include <list>

using namespace std;

class XSocketInterface : public XSocket
{
public:
    XSocketInterface();
    XSocketInterface(XSocket & sx)
    {
        remotePair = sx.getRemotePair();
        useWrite = false;
        sockfd = sx.GetSocket();
    }


    bool WriteBlock(const void * data, uint32_t datalen);
    bool ReadBlock(void * data, uint32_t datalen, bool bestrict = false);
    void * ReadBlockAlloc(unsigned int * datalen);

    list<string> ReadStringList(bool *bok=NULL, unsigned int max=1048576);
    bool WriteStringList(const list<string> & val,unsigned int max=1048576);

    string ReadStringLow();
    bool WriteStringLow(const string & val);

    string ReadString(unsigned int max=1048576, bool * okx=NULL);
    bool WriteString(const string & val,unsigned int max=1048576);

    unsigned char ReadUChar(bool *bok=NULL);
    bool WriteUChar(const unsigned char & c);

    unsigned int ReadUInt32(bool *bok=NULL);
    bool WriteUInt32(const unsigned int & c);

    int ReadInt32(bool *bok=NULL);
    bool WriteInt32(const int & c);
};

#endif // XSOCKETINTERFACE_H
