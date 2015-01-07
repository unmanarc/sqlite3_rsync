#ifndef XSocket_H
#define XSocket_H

#include <string>
#include <stdint.h>

using namespace std;

class XSocket {
public:
    XSocket();
    virtual ~XSocket();

    void Close();

    XSocket(const int & sx);
    XSocket & operator=(const int & sx);

    void SetUseWrite();
    void SetTCPNODELAY();
    void SetNoBrokenPipe();

    void SetRecvBuffer(int xsize);

    virtual bool IsConnected();
    virtual bool Connect(const string & hostname, const  uint16_t port, uint32_t timeout);
    void TryConnect(const string & hostname, const  uint16_t port, uint32_t timeout);
    virtual bool Listen(uint16_t port, const string & listenOnAddr);
    virtual XSocket Accept();
    string LastError() const;

    virtual bool Write(const void * data, uint32_t datalen);
    virtual bool Read(void * data, uint32_t datalen);

    uint16_t GetPort();
    bool SetReadTimeout(unsigned int toval);

    bool IsSocketSettedUp() const;

    void SetSocket(int _sockfd);
    int GetSocket() const;

    string getRemotePair() const;
    void setRemotePair(const string &value);

protected:
    string remotePair;

    bool useWrite;
    string lastError;
    int sockfd;
};



#endif // XSocket_H
