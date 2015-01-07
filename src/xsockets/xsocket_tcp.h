#ifndef XSocket_TCP_H
#define XSocket_TCP_H

#include "xsocket.h"

#include <stdint.h>
#include <string>

using namespace std;

class XTCPSocket: public XSocket
{
public:
	XTCPSocket();

	bool IsConnected();

	bool Listen(uint16_t port, const string & listenOnAddr = "::", bool useIPv4 = false, int recvbuffer = 0);
	bool Connect(const string & hostname, uint16_t port, uint32_t timeout = 0);
	XSocket Accept();
	bool ThreadedAcceptTCP(bool (*func)(void *, XTCPSocket &), void * obj);
};

struct callbackFunction
{
	XTCPSocket * caller;
	bool (*func)(void *, XTCPSocket &);
	void * obj;
};

#endif // XSocket_TCP_H
