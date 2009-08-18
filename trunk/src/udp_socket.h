#ifndef UDP_SOCKET_H
#define UDP_SOCKET_H

#include "net_socket.h"
#include <vector>

class UdpSocket: public IONetSocket {
  private :
    size_t maxSize;

    friend class UdpAddress;

    /*ssize_t readData(   char *buffer, size_t size, int flags,
                        NetAddress *addr);*/
    ssize_t readRawData(char *buffer, size_t size, int flags,
                        NetAddress *addr);
    ssize_t peekData(   char *buffer, size_t size,
                        NetAddress *addr);
    ssize_t writeData(  const char *data, size_t size, int flags,
                        const NetAddress *addr);
    ssize_t writeData(  const struct iovec *iov, int iovcnt,
                        const NetAddress *addr);
  public :
    UdpSocket();
    UdpSocket(int localPort);

    void setMaximumSize(size_t maxSize);
    size_t getMaximumSize(void);

    ssize_t writeObject(const Serializable &object, const NetAddress &addr);
    ssize_t writeString(const std::string &string, const NetAddress &addr);
    ssize_t writeBytes(const Buffer<char> &data, const NetAddress &addr, int flags = 0);

    Buffer<char> *readBytes(Buffer<char> *buff, NetAddress *addr, int flags = 0);
    Buffer<char> *readBytes(Buffer<char> *buff, NetAddress *addr, uint64_t nanosec, int flags = 0);
    Buffer<char> *readBytes(Buffer<char> *buff, NetAddress *addr, time_t sec, long nanosec, int flags = 0);
    String *readString(NetAddress *addr);
    String *readString(NetAddress *addr, uint64_t nanosec);
    String *readString(NetAddress *addr, time_t sec, long nanosec);
    Serializable *readObject(NetAddress *addr);
    Serializable *readObject(NetAddress *addr, uint64_t nanosec);
    Serializable *readObject(NetAddress *addr, time_t sec, long nanosec);
};

#endif
