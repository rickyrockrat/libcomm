#include <sys/socket.h>  // socket()
#include <netdb.h>       // gethostbyname(), htons()
#include <string.h> 	// memcpy
#include <stdlib.h> //free
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>

#include <iostream>

#include "udp_socket.h"
#include "serialization_manager.h"


//TODO Add security when wrong udp packets!

#define bufferSize 65535
#define NB_USEC_PER_SEC 1000000000

UdpSocket::UdpSocket(): NetSocket(SOCK_DGRAM) {}

UdpSocket::UdpSocket(int localPort): NetSocket(SOCK_DGRAM) {
  bindSocket(localPort);
}

void UdpSocket::sendObject(const Serializable &object, 
  const NetAddress &address) {
  struct sockaddr_in clientAddr;
  socklen_t size;
  SerializationManager *serManager;
  size_t sizeData = 0;
  char *data;
  NetMessage *message = (NetMessage*) NULL;
  
  address.getSockAddr(&clientAddr);
  size = sizeof(clientAddr);
  
  serManager = SerializationManager::getSerializationManager();
  message = serManager->checkAndSerialize(object, NULL);

  if (message == (NetMessage*) NULL) {
    std::string errorMessage = "Object size bigger than udp max data size.";
    delete message;
    throw (UdpSocket::NetException(EMSGSIZE, errorMessage));
  } else {
    data = message->getRawData(true,&sizeData);
    ssize_t sizeSent = sendto(socketId, data, sizeData, 0,
      (sockaddr*) &clientAddr, size);
    free(data);
    delete message;
  
    if (sizeSent == -1) {
      launchNetExceptionForSendReceive(errno);
    }
  }

}

Serializable *UdpSocket::receiveObject(NetAddress *address) {
  struct sockaddr_in clientAddr;
  socklen_t sizeAddr;
  char* buff = new char[bufferSize];
  NetMessage *udpMessage = (NetMessage*) NULL;
  
  sizeAddr = sizeof(clientAddr);
  
  ssize_t sizeReceived = 
    recvfrom (socketId, buff, bufferSize, 0, (sockaddr*) &clientAddr, &sizeAddr);
  
  if (sizeReceived == -1) {
    delete [] buff;
    launchNetExceptionForSendReceive(errno);
  }
  udpMessage = new NetMessage(buff, sizeReceived, NULL, true);
    
  if (address != NULL) {
    *address = NetAddress(clientAddr);
  }
    
  SerializationManager *serManager =
    SerializationManager::getSerializationManager();
  void *object = serManager->deserialize(*udpMessage, false);
   
  delete [] buff;
  delete udpMessage;
  return (Serializable*) object;
}

Serializable *UdpSocket::receiveObject(NetAddress *address, uint64_t nanosec) {
  time_t sec = nanosec / NB_USEC_PER_SEC;
  long nsec = nanosec % NB_USEC_PER_SEC;
  return receiveObject(address,sec,nsec);
}

Serializable *UdpSocket::receiveObject(NetAddress *address, time_t sec,
  long nanosec) {
  struct sockaddr_in clientAddr; 
  socklen_t sizeAddr;
  char* buff = (char*) NULL;
  NetMessage *udpMessage = (NetMessage*) NULL;
  timespec timeout; 
  fd_set fds;

  sizeAddr = sizeof(clientAddr);

  timeout.tv_sec = sec; 
  timeout.tv_nsec = nanosec;  

  FD_ZERO(&fds);
  FD_SET(socketId,&fds);

  int resultSelect = pselect(socketId+1, &fds, NULL, NULL, &timeout,NULL);
  if (resultSelect == -1) {
      launchNetExceptionForSelect(errno);
  } else if (resultSelect == 0) {
    throw (NetSocket::NetException(SOCKET_TIMEOUT,"Timeout on receive occurs."));
  } else {
    buff = new char[bufferSize];
    ssize_t sizeReceived = 
      recvfrom (socketId, buff, bufferSize, 0, (sockaddr*) &clientAddr, &sizeAddr);
  
    if (sizeReceived == -1) {
      delete [] buff;
      launchNetExceptionForSendReceive(errno);
    }

    try {
      udpMessage = new NetMessage(buff, sizeReceived, NULL, true);
    } catch (Exception &e) {
      delete [] buff;
      throw e;
    }
  }
    
  if (address != NULL) {
    *address = NetAddress(clientAddr);
  }
    
  SerializationManager *serManager =
    SerializationManager::getSerializationManager();
  void *object = serManager->deserialize(*udpMessage, false);
   
  delete [] buff;
  delete udpMessage;
  return (Serializable*) object;
}


Serializable *UdpSocket::receiveObject(NetAddress *address, const std::vector<UdpSocket*> &sockets,
  timespec *timeout, UdpSocket **read_from) {
  
  struct sockaddr_in clientAddr;
  socklen_t sizeAddr;
  char* buff = new char[bufferSize];
  NetMessage *udpMessage = (NetMessage*) NULL;
  std::map<int, UdpSocket*> sockets_map;
  fd_set fds;
  size_t i;
  UdpSocket *socket = NULL;
  int highestSocketId = 0;
  
  sizeAddr = sizeof(clientAddr);
  
  FD_ZERO(&fds);

  for (i = 0; i<sockets.size(); ++i) {
    socket = sockets[i];
    
    sockets_map[socket->socketId] = socket;
    FD_SET(socket->socketId,&fds);
    if (highestSocketId < socket->socketId) highestSocketId = socket->socketId;
  }

  int resultSelect = pselect(highestSocketId+1, &fds, NULL, NULL, NULL, NULL);
  
  if (resultSelect == -1) {
    launchNetExceptionForSelect(errno);
  } else if (resultSelect == 0) {
      throw (NetSocket::NetException(SOCKET_TIMEOUT,"Timeout on receive occurs."));
  } else {
    socket = sockets_map[resultSelect];
    if (socket != NULL) {
      ssize_t sizeReceived = 
        recvfrom (socket->socketId, buff, bufferSize, 0, (sockaddr*) &clientAddr, &sizeAddr);
        
      if (sizeReceived == -1) {
        delete [] buff;
        launchNetExceptionForSendReceive(errno);
      }

      try {
        udpMessage = new NetMessage(buff, sizeReceived, NULL, true);
      } catch (Exception &e) {
        delete [] buff;
        throw e;
      }
    }
  }

  if (address != NULL) {
    *address = NetAddress(clientAddr);
  }
    
  *read_from = socket;

  SerializationManager *serManager =
    SerializationManager::getSerializationManager();
  void *object = serManager->deserialize(*udpMessage, false);
   
  delete [] buff;
  delete udpMessage;
  return (Serializable*) object;
}

Serializable *UdpSocket::receiveObject(NetAddress *address, const std::vector<UdpSocket*> &sockets,
  UdpSocket **read_from) {
  
  return receiveObject(address, sockets, (timespec*) NULL, read_from);
}

Serializable *UdpSocket::receiveObject(NetAddress *address, const std::vector<UdpSocket*> &sockets,
  uint64_t nanosec, UdpSocket **read_from) {
  
  time_t sec = nanosec / NB_USEC_PER_SEC;
  long nsec = nanosec % NB_USEC_PER_SEC;
  return receiveObject(address, sockets, sec, nsec, read_from);
}

Serializable *UdpSocket::receiveObject(NetAddress *address, const std::vector<UdpSocket*> &sockets,
  time_t sec, long nanosec, UdpSocket **read_from) {
  
  timespec timeout; 

  timeout.tv_sec = sec; 
  timeout.tv_nsec = nanosec;  
  return receiveObject(address, sockets, &timeout, read_from);
}
