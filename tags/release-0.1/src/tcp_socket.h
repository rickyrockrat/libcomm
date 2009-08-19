#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

#include "net_socket.h"
#include "structs/buffer_serializable.h"

#include <vector>

class TcpSocket : public NetSocket {
  private :
    char *resiliantData;
    size_t resiliantDataSize;
    size_t lastTestedIndex;

    TcpSocket(int sockedId);
    NetMessage *tryBuildObject(char *&buff, size_t &totalSize);
    std::string *tryBuildString(char *&buff, size_t &totalSize);
    bool containsCRLF(char *buff, size_t size, size_t *index);
    void receiveData(char *&buff, size_t &totalSize);
    static void launchNetExceptionForGetPeerName(int code);
    static void launchNetExceptionForConnect(int code);
    static void launchNetExceptionForAccept(int code);

    friend class TcpServerSocket;
  public :
    TcpSocket();
    TcpSocket(const NetAddress &address);
    virtual ~TcpSocket();
    void shutdownSocket(bool read, bool write);
    void connectSocket(const NetAddress &address);
    void connectSocket(const NetAddress &address, uint64_t nanosec);
    void connectSocket(const NetAddress &address, time_t sec, long nanosec);
    void sendObject(const Serializable &object);
    void sendString(const std::string &string);
    void sendBytes(const Buffer<char> &data);
    Serializable *receiveObject();
    Serializable *receiveObject(uint64_t nanosec);
    Serializable *receiveObject(time_t sec, long nanosec);
    static Serializable *receiveObject(const std::vector<TcpSocket*> &sockets,
      int *socket_index);
    static Serializable *receiveObject(const std::vector<TcpSocket*> &sockets,
      uint64_t nanosec, int *socket_index);
    static Serializable *receiveObject(const std::vector<TcpSocket*> &sockets,
      time_t sec, long nanosec, int *socket_index);
    std::string *receiveString(void);
    Buffer<char> *receiveBytes(void);

    NetAddress getDistantAddress() const;
  
};

class TcpServerSocket : public NetSocket {
  
  private:
    void listenTo(int backlog);
    static void launchNetExceptionForListen(int code);
    static void launchNetExceptionForAccept(int code);

  public :
    TcpServerSocket();
    TcpServerSocket(int localPort);
    TcpServerSocket(int localPort, int backlog);
    TcpSocket *acceptConnection();
    TcpSocket *acceptConnection(uint64_t nanosec);
    TcpSocket *acceptConnection(time_t sec, long nanosec);
};

#endif
