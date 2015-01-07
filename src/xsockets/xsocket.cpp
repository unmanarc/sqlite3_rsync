#include "xsocket.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/tcp.h>

XSocket::XSocket()
{
	sockfd = 0;
	useWrite = false;
	lastError = "";
}

XSocket::~XSocket()
{
	//Close();
}

XSocket::XSocket(const int &sx)
{
	useWrite = false;
	sockfd = sx;
}

XSocket & XSocket::operator=(const int & sx)
{
	useWrite = false;
	sockfd = sx;
	return *this;
}

void XSocket::SetUseWrite()
{
	signal(SIGPIPE, SIG_IGN);
	useWrite = true;
}

void XSocket::SetTCPNODELAY()
{
	int flag = 1;
	setsockopt(sockfd, /* socket affected */
	IPPROTO_TCP, /* set option at TCP level */
	TCP_NODELAY, /* name of option */
	(char *) &flag, /* the cast is historical cruft */
	sizeof(int)); /* length of option value */
}

void XSocket::SetRecvBuffer(int xsize)
{
	int buffsize = xsize;
	setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &buffsize, sizeof(buffsize));
}

bool XSocket::IsConnected()
{
	return false;
}

bool XSocket::Connect(const std::string &, uint16_t, uint32_t)
{
	return false;
}

void XSocket::TryConnect(const string &hostname, const uint16_t port, uint32_t timeout)
{
	while (!Connect(hostname, port, timeout))
	{
		// Try to reconnect if fail...
	}
}

bool XSocket::Listen(uint16_t, const string &)
{
	return false;
}

XSocket XSocket::Accept()
{
	XSocket sk;
	return sk;
}

bool XSocket::Write(const void *data, uint32_t datalen)
{
	if (!useWrite)
	{
		int32_t j, c = 0;
		int32_t tl = datalen;

		if (!datalen)
			return true;

		// Send the raw data.
		while ((j = send(sockfd, (char *) data + (datalen - tl), datalen,
		MSG_NOSIGNAL)) < tl && c < 5)
		{
			if (j == -1)
			{
				// if send failed hard, get away-
				shutdown(sockfd, 2);
				return false;
			}
			// or just count the success.
			else
				tl -= j;

			// count the attempt.
			c++;
		}

		// Failed to achieve sending the contect on 5 attempts
		if (c == 5 && tl != 0)
		{
			shutdown(sockfd, 2);
			return false;
		}
		return true;
	}
	else
	{
		int32_t j, c = 0;
		int32_t tl = datalen;

		if (!datalen)
			return true;

		// Send the raw data.
		while ((j = write(sockfd, (char *) data + (datalen - tl), datalen)) < tl && c < 5)
		{
			if (j == -1)
			{
				// if send failed hard, get away-
				shutdown(sockfd, 2);
				return false;
			}
			// or just count the success.
			else
				tl -= j;

			// count the attempt.
			c++;
		}

		// Failed to achieve sending the contect on 5 attempts
		if (c == 5 && tl != 0)
		{
			shutdown(sockfd, 2);
			return false;
		}
		return true;
	}
}

bool XSocket::Read(void *data, uint32_t datalen)
{
	if (!useWrite)
	{
		int32_t recvlenght = 0;
		int32_t recvlocallenght = 0;

		if (!datalen)
			return true;

		// Try to receive the maximum amount of data left.
		while ((recvlocallenght = recv(sockfd, ((char *) data) + recvlenght, datalen - recvlenght, 0)) != -1 && recvlocallenght)
		{
			// Count the data received.
			recvlenght += recvlocallenght;
			// If finished, then done.
			if ((uint32_t) recvlenght >= datalen)
				return true;
			// Otherwise continue receiving.
		}

		// if not received anything, or some shit happened, bye with false
		if (!recvlocallenght || recvlocallenght == -1)
			return false;

		// Bye with true.
		return true;
	}
	else
	{
		int32_t recvlenght = 0;
		int32_t recvlocallenght = 0;

		if (!datalen)
			return true;

		// Try to receive the maximum amount of data left.
		while ((recvlocallenght = read(sockfd, ((char *) data) + recvlenght, datalen - recvlenght)) != -1 && recvlocallenght)
		{
			// Count the data received.
			recvlenght += recvlocallenght;
			// If finished, then done.
			if ((uint32_t) recvlenght >= datalen)
				return true;
			// Otherwise continue receiving.
		}

		// if not received anything, or some shit happened, bye with false
		if (!recvlocallenght || recvlocallenght == -1)
			return false;

		// Bye with true.
		return true;
	}
}

uint16_t XSocket::GetPort()
{
	struct sockaddr_in sin;
	socklen_t len = sizeof(sin);
	if (getsockname(sockfd, (struct sockaddr *) &sin, &len) == -1)
	{
		lastError = "Error resolving port";
		return 0;
	}
	return ntohs(sin.sin_port);
}

void XSocket::Close()
{
	if (sockfd <= 0)
		return;
	shutdown(sockfd, SHUT_RDWR);
	close(sockfd);
	sockfd = 0;
}

bool XSocket::SetReadTimeout(unsigned int toval)
{
	struct timeval timeout;
	timeout.tv_sec = toval;
	timeout.tv_usec = 0;
	if ((setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout))) == -1)
	{
		return false;
	}
	return true;
}

bool XSocket::IsSocketSettedUp() const
{
	return (sockfd != 0);
}

int XSocket::GetSocket() const
{
	return sockfd;
}
string XSocket::getRemotePair() const
{
	return remotePair;
}

void XSocket::setRemotePair(const string &value)
{
	remotePair = value;
}

