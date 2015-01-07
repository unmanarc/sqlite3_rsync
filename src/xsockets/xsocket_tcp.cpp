#include "xsocket_tcp.h"
#include <string.h>
#include <arpa/inet.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

XTCPSocket::XTCPSocket()
{
}

bool XTCPSocket::Connect(const std::string & hostname, uint16_t port,
		uint32_t timeout)
{
	if (sockfd)
		Close(); // close first

	char servport[16];
	int rc;
	struct in6_addr serveraddr;
	struct addrinfo hints, *res = NULL;

	memset(&hints, 0x00, sizeof(hints));
	hints.ai_flags = AI_NUMERICSERV;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	rc = inet_pton(AF_INET, hostname.c_str(), &serveraddr);
	if (rc == 1) /* valid IPv4 text address? */
	{
		hints.ai_family = AF_INET;
		hints.ai_flags |= AI_NUMERICHOST;
	}
	else
	{
		rc = inet_pton(AF_INET6, hostname.c_str(), &serveraddr);
		if (rc == 1) /* valid IPv6 text address? */
		{
			hints.ai_family = AF_INET6;
			hints.ai_flags |= AI_NUMERICHOST;
		}
	}

	snprintf(servport, 16, "%u", port);

	rc = getaddrinfo(hostname.c_str(), servport, &hints, &res);
	if (rc != 0)
	{
		// Host not found.
		lastError = "getaddrinfo() failed";
		return false;
	}

	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sockfd < 0)
	{
		lastError = "socket() failed";
		return false;
	}

	// Set the timeout here.
	SetReadTimeout(timeout);

	bool connected = false;

	for (struct addrinfo *resiter = res; resiter; resiter = resiter->ai_next)
	{
		if ((rc = connect(sockfd, resiter->ai_addr, resiter->ai_addrlen)) >= 0)
		{
			connected = true;
			break;
		}
	}

	freeaddrinfo(res);

	if (!connected)
	{
		lastError = "connect() failed";
		return false;
	}

	return true;
}

void *TCPAcceptorThread(void *d)
{
	callbackFunction * cbf = (callbackFunction *) d;

	XTCPSocket ks;
	do
	{
		// ACCEPT A CONNECTION
		XSocket s = cbf->caller->Accept();
		ks.SetSocket(s.GetSocket());
		// if valid: callback.
		if (ks.IsSocketSettedUp())
		{
			cbf->func(cbf->obj, ks);
		}
		// Continue for valid connections
	} while (ks.IsSocketSettedUp());

	fprintf(stderr, "TCPAcceptorThread died unexpectedly.\n");
	fflush(stdout);

	delete cbf;

	pthread_exit(NULL);
}

bool XTCPSocket::ThreadedAcceptTCP(bool (*func)(void *, XTCPSocket &),
		void * obj)
{
	pthread_t p_thread_db;
	callbackFunction * cbf = new callbackFunction;

	cbf->func = func;
	cbf->obj = obj;
	cbf->caller = this;

	pthread_create(&p_thread_db, NULL, TCPAcceptorThread, cbf);

	return true;
}

std::string XSocket::LastError() const
{
	return lastError;
}

void XSocket::SetSocket(int _sockfd)
{
	sockfd = _sockfd;
}

XSocket XTCPSocket::Accept()
{
	int sdconn;
	XSocket cursocket;
	cursocket.SetSocket(0);

	int32_t clilen;
	struct sockaddr_in cli_addr;
	clilen = sizeof(cli_addr);

	if ((sdconn = accept(sockfd, (struct sockaddr *) &cli_addr,
			(socklen_t *) &clilen)) >= 0)
	{
		// Set the proper socket-
		cursocket.SetSocket(sdconn);
		char ipAddr[80];
		inet_ntop(AF_INET, &cli_addr.sin_addr, ipAddr, sizeof(ipAddr));

		cursocket.setRemotePair(ipAddr);
	}
	// Establish the error.
	else
		lastError = "accept() failed";

	// return the socket class.
	return cursocket;
}

bool XTCPSocket::Listen(uint16_t port, const string & listenOnAddr,
		bool useIPv4, int recvbuffer)
{
	int on = 1;

	if (useIPv4)
	{
		if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
			lastError = "socket() failed";
			return false;
		}
	}
	else
	{
		if ((sockfd = socket(AF_INET6, SOCK_STREAM, 0)) < 0)
		{
			lastError = "socket() failed";
			return false;
		}
	}

	if (recvbuffer)
		SetRecvBuffer(recvbuffer);

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on))
			< 0)
	{
		lastError = "setsockopt(SO_REUSEADDR) failed";
		Close();
		return false;
	}
	if (useIPv4)
	{
		struct sockaddr_in serveraddr;
		memset(&serveraddr, 0, sizeof(serveraddr));
		serveraddr.sin_family = AF_INET;
		serveraddr.sin_port = htons(port);
		inet_pton(AF_INET, listenOnAddr.c_str(), &serveraddr.sin_addr);
		if (bind(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr))
				< 0)
		{
			fprintf(stderr, "bind() failed for %s:%d ", listenOnAddr.c_str(),
					port);
			perror("");
			lastError = "bind() failed";
			Close();
			return false;
		}
	}
	else
	{
		struct sockaddr_in6 serveraddr;
		memset(&serveraddr, 0, sizeof(serveraddr));
		serveraddr.sin6_family = AF_INET6;
		serveraddr.sin6_port = htons(port);
		//serveraddr.sin6_addr   = in6addr_any;
		inet_pton(AF_INET6, listenOnAddr.c_str(), &serveraddr.sin6_addr);
		if (bind(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr))
				< 0)
		{
			lastError = "bind() failed";
			Close();
			return false;
		}
	}
	if (listen(sockfd, 10) < 0)
	{
		lastError = "bind() failed";
		Close();
		return false;
	}
	return true;
}

bool XTCPSocket::IsConnected()
{
	if (sockfd <= 0)
		return false;

	struct sockaddr peer;
	socklen_t peer_len;
	/* We must put the length in a variable.              */
	peer_len = sizeof(peer);
	/* Ask getpeername to fill in peer's socket address.  */
	if (getpeername(sockfd, &peer, &peer_len) == -1)
	{
		// No peer name... so... not connected.
		return false;
		sockfd = 0;
	}
	return true;
}
