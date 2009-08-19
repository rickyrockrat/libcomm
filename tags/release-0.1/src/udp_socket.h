#ifndef UDP_SOCKET_H
#define UDP_SOCKET_H

#include <netinet/in.h>  // sockaddr_in
#include <string>
#include <vector>

#include "serializable.h"
#include "net_address.h"
#include "net_socket.h"
#include "exception.h"

class UdpSocket: public NetSocket {
  
  private :
    friend class UdpAddress;

    static Serializable *receiveObject(NetAddress *address, const std::vector<UdpSocket*> &sockets,
      timespec *timeout, UdpSocket **read_from);

  public :
    UdpSocket();
    UdpSocket(int localPort);
    void sendObject(const Serializable &object, const NetAddress &address);
    Serializable *receiveObject(NetAddress *address);
    Serializable *receiveObject(NetAddress *address, uint64_t nanosec);
    Serializable *receiveObject(NetAddress *address, time_t sec, long microsec);
    static Serializable *receiveObject(NetAddress *address, const std::vector<UdpSocket*> &sockets,
      UdpSocket **read_from);
    static Serializable *receiveObject(NetAddress *address, const std::vector<UdpSocket*> &sockets,
      uint64_t nanosec, UdpSocket **read_from);
    static Serializable *receiveObject(NetAddress *address, const std::vector<UdpSocket*> &sockets,
      time_t sec, long nanosec, UdpSocket **read_from);

};

#endif
