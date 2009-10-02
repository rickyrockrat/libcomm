#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

#include "net_socket.h"
#include <vector>

class TcpSocket : public IONetSocket {
  private :

    TcpSocket(int sockedId);

    ssize_t readRawData(char *buffer, size_t size, int flags,
                        NetAddress *addr);
    ssize_t writeData(  const char *data, size_t size, int flags,
                        const NetAddress *addr);
    ssize_t writeData(  const struct iovec *iov, int iovcnt,
                        const NetAddress *addr);

    friend class TcpServerSocket;
  public :
    TcpSocket(void);
    TcpSocket(const NetAddress &address);
    virtual ~TcpSocket();

    void shutdownSocket(bool read, bool write);
};

class TcpServerSocket : public NetSocket {
  
  private:
    int socketId;

    void listenTo(int backlog);
    int getSocketId(void) const;

  public :
    TcpServerSocket();
    TcpServerSocket(int localPort);
    TcpServerSocket(int localPort, int backlog);
    TcpSocket *acceptConnection();
    TcpSocket *acceptConnection(uint64_t nanosec);
    TcpSocket *acceptConnection(time_t sec, long nanosec);

    void closeServer(void);
};

#endif
