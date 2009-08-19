#include <sys/types.h>
#include <sys/socket.h>  
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h> //free
#include <string.h>

#include "tcp_socket.h"
#include "serialization_manager.h"
#include "timer.h"

#define bufferSize 4096
#define DEFAULT_BACKLOG 15
#define NB_USEC_PER_SEC 1000000000


TcpSocket::TcpSocket(int socketId): NetSocket(),
  resiliantData(NULL), resiliantDataSize(0), lastTestedIndex(0) {
  this->socketId = socketId;
}

TcpSocket::TcpSocket(): NetSocket(SOCK_STREAM), 
  resiliantData(NULL), resiliantDataSize(0), lastTestedIndex(0) {
}

TcpSocket::TcpSocket(const NetAddress &address): NetSocket(SOCK_STREAM),
  resiliantData(NULL), resiliantDataSize(0), lastTestedIndex(0) {
  connectSocket(address);
}

TcpSocket::~TcpSocket() {
  if (resiliantData != NULL) free(resiliantData);
}

void TcpSocket::shutdownSocket(bool read, bool write) {
  int result;
  int how = ((read) ? ((write) ? SHUT_RDWR : SHUT_RD) : (SHUT_WR));

  result = shutdown(socketId, how);
  if (result == -1) {
    std::string errorMessage;
    switch (errno) {
      case EBADF:  
        errorMessage = "s is not a valid descriptor.";
        break;
      case ENOTCONN:
        errorMessage = "The specified socket is not connected.";
        break;
      case ENOTSOCK:
        errorMessage = "s is a file, not a socket.";
        break;
    }
    throw (NetAddress::NetException(errno,errorMessage));
  }
}

void TcpSocket::connectSocket(const NetAddress &address) {
  struct sockaddr_in distAddr;
  int result;
  
  address.getSockAddr(&distAddr);
  result = connect(socketId, (sockaddr*)&distAddr, sizeof(distAddr));
  if (result == -1) {
    launchNetExceptionForConnect(errno); 
  }
}

void TcpSocket::connectSocket(const NetAddress &address, uint64_t nanosec) {
  time_t sec = nanosec / NB_USEC_PER_SEC;
  long nsec = nanosec % NB_USEC_PER_SEC;
  return connectSocket(address,sec,nsec);
}
void TcpSocket::connectSocket(const NetAddress &address, time_t sec, long nanosec) {
  int result; 
  fd_set fdsWrite;
  fd_set fdsError;
  timespec timeout; 
  long flags;
  
  flags = fcntl(socketId, F_GETFL, NULL);
  result = fcntl(socketId, F_SETFL, flags | O_NONBLOCK);
  if (result != 0) {
    throw NetSocket::NetException(errno, "Impossible to set the socket in no"
      "n blocking state. It is necessary to apply timeout on connect");
  }

  try {
    connectSocket(address);
  } catch (Exception &e) {
    if (e.getCode() != EINPROGRESS) {
      throw e;
    } else {
      timeout.tv_sec = sec; 
      timeout.tv_nsec = nanosec;  

      FD_ZERO(&fdsWrite);
      FD_SET(socketId,&fdsWrite);
      FD_ZERO(&fdsError);
      FD_SET(socketId,&fdsError);

      result = pselect(socketId+1, NULL, &fdsWrite, &fdsError, &timeout, NULL);
      if (result == 0) {
        throw (NetSocket::NetException(SOCKET_TIMEOUT,
          "Timeout on connect occurs."));
      } else if (result == -1) {
        launchNetExceptionForSelect(errno);
      } else {
        result = fcntl(socketId, F_SETFL, flags);
        if (result != 0) {
          throw NetSocket::NetException(errno, "Impossible to set the socket i"
            "n its orginal state.");
        } else {
          try {
            if (getDistantAddress().getAddress() == "0.0.0.0") {
              throw (NetSocket::NetException(SOCKET_CONNECTION_FAILURE,
                "Error when connecting the socket."));
            }
          } catch (Exception &e) {
            switch (e.getCode()) {
              case ENOTCONN:
                throw (NetSocket::NetException(SOCKET_CONNECTION_FAILURE,
                  "Error when connecting the socket."));
                break;
              default:
                throw e;
                break;
            }
          }
        }
      }
    }
  }
}

void TcpSocket::sendObject(const Serializable &object) {
  NetMessage *message;
  SerializationManager *serManager;
  char *data;
  size_t sizeData = 0;
  ssize_t sizeSent;
  size_t totalDataSent = 0;
  
  serManager = SerializationManager::getSerializationManager();
  message = serManager->serialize(object, NULL);
  data = message->getRawData(true,&sizeData);

  while (totalDataSent < sizeData) {
    sizeSent = send(socketId, &(data)[totalDataSent], sizeData - totalDataSent, 0);
    if (sizeSent == -1) {
      free(data);
      delete message;
      launchNetExceptionForSendReceive(errno);
    } else {
      totalDataSent += sizeSent;
    }
  }

  free(data);
  delete message;
}

//TODO check sizeSent to see if all has been sent
void TcpSocket::sendString(const std::string &string) {
  std::string toSend = string;
  toSend += "\r\n";
  ssize_t sizeSent = send(socketId, toSend.c_str(), toSend.size(), 0);

  if (sizeSent == -1) {
    launchNetExceptionForSendReceive(errno);
  }
}

void TcpSocket::sendBytes(const Buffer<char> &data) {
  ssize_t sizeSent = send(socketId, data.data(), data.size(), 0);

  if (sizeSent == -1) {
    launchNetExceptionForSendReceive(errno);
  }
}

NetMessage *TcpSocket::tryBuildObject(char *&buff, size_t &totalSize) {
  NetMessage *tcpMessage = (NetMessage*) NULL;

  try { 
    tcpMessage = new NetMessage(buff, totalSize, NULL, true);
    size_t totalMessageSize = tcpMessage->getDataSize()
      + Serializable::sizeOfDataHeadersWithTs;
    if (totalSize > totalMessageSize) {
      resiliantDataSize = totalSize-totalMessageSize; 
      resiliantData = (char*) malloc(resiliantDataSize*sizeof(char));
      memcpy(resiliantData, &(buff[totalMessageSize]),resiliantDataSize);
    }
  } catch (Exception &e) {
    switch (e.getCode()) {
      case NET_MESSG_NEDFC:
      case NET_MESSG_NEDFH:
        buff = (char*) realloc(buff,(totalSize+bufferSize)*sizeof(char));
        break;
      default:
        throw e;
        break;
    }
  }

  return tcpMessage;
}

std::string *TcpSocket::tryBuildString(char *&buff, size_t &totalSize) {
  std::string *string = NULL;
  size_t index;
  size_t stringSize;
  
  if (containsCRLF(&(buff[lastTestedIndex]), totalSize, &index)) {
    stringSize = lastTestedIndex + index;
    lastTestedIndex = 0;
    if (totalSize > stringSize) {
      resiliantDataSize = totalSize-stringSize; 
      resiliantData = (char*) malloc(resiliantDataSize*sizeof(char));
      memcpy(resiliantData, &(buff[stringSize]),resiliantDataSize);
    }
    string = new std::string(buff,stringSize-2);
  } else {
    lastTestedIndex = totalSize;
  }
  return string;
}

bool TcpSocket::containsCRLF(char *buff, size_t size, size_t *index) {
  size_t i;

  for (i = 0; i < size; ++i) {
    if (buff[i] == '\r') {
      if (i < (size-1)) {
        if (buff[i+1] == '\n') {
          *index = i+2;
          return true;
        }
      }
    }
  }
  return false;
}

void TcpSocket::receiveData(char *&buff, size_t &totalSize) {
  if (resiliantData != NULL) {
    totalSize = resiliantDataSize;
    buff = (char*) malloc(totalSize*sizeof(char));
    memcpy(buff,resiliantData,resiliantDataSize);
    free(resiliantData);
    resiliantData = NULL;
    resiliantDataSize = 0;
  } else {
    if (buff == NULL) {
      buff = (char*) malloc(bufferSize*sizeof(char));
    }
    ssize_t sizeReceived = 
      recv(socketId, &(buff[totalSize]), bufferSize, 0);
    if (sizeReceived == -1) {
      free(buff);
      launchNetExceptionForSendReceive(errno);
    } else if (sizeReceived == 0) {
      throw NetSocket::NetException(SOCKET_CLOSED, 
        "Distant socket has been closed.");
    }
    totalSize += sizeReceived;
  }
}

#include <iostream>
Serializable *TcpSocket::receiveObject() {
  char* buff = NULL;
  size_t totalSize = 0;
  NetMessage *tcpMessage = (NetMessage*) NULL;
  
  while (tcpMessage == NULL) {
    receiveData(buff, totalSize);
    tcpMessage = tryBuildObject(buff,totalSize);
  }
    
  SerializationManager *serManager =
    SerializationManager::getSerializationManager();
  void *object = serManager->deserialize(*tcpMessage, false);
   
  free(buff);
  delete tcpMessage;
  return (Serializable*) object;
}

Serializable *TcpSocket::receiveObject(uint64_t nanosec) {
  time_t sec = nanosec / NB_USEC_PER_SEC;
  long nsec = nanosec % NB_USEC_PER_SEC;
  return receiveObject(sec,nsec);
}

Serializable *TcpSocket::receiveObject(time_t sec, long nanosec) {
  char* buff = NULL;
  size_t totalSize = 0;
  bool again = true;
  NetMessage *tcpMessage = (NetMessage*) NULL;
  timespec timeout; 
  fd_set fds;
  Timer timer;
  time_t sec_elasped;
  long nanosec_elapsed;

  timeout.tv_sec = sec; 
  timeout.tv_nsec = nanosec;  

  FD_ZERO(&fds);
  FD_SET(socketId,&fds);

  timer.startTimer();

  do {
    int resultSelect;
    
    resultSelect = pselect(socketId+1, &fds, NULL, NULL, &timeout,NULL);
    if (resultSelect == -1) {
      launchNetExceptionForSelect(errno);
    } else if (resultSelect == 0) {
      throw (NetException::NetException(SOCKET_TIMEOUT,"Timeout on receive occurs."));
      if (buff != NULL) {
        resiliantDataSize = totalSize; 
        resiliantData = (char*) malloc(resiliantDataSize*sizeof(char));
        memcpy(resiliantData, buff,resiliantDataSize);
      }
    } else {
      receiveData(buff, totalSize);
      tcpMessage = tryBuildObject(buff, totalSize);
      if (tcpMessage == NULL) {
        timer.getDiffTime(sec_elasped,nanosec_elapsed,true);
        computeTimeDiff(sec,nanosec, sec_elasped, nanosec_elapsed,
          timeout.tv_sec, timeout.tv_nsec);
      } else {
        again = false;
      }
    }
  } while (again);
  
  SerializationManager *serManager =
    SerializationManager::getSerializationManager();
  void *object = serManager->deserialize(*tcpMessage, false);
   
  free(buff);
  delete tcpMessage;
  return (Serializable*) object;

}

#include "logger.h"

Serializable *TcpSocket::receiveObject(const std::vector<TcpSocket*> &sockets, 
  int *socket_index) {
  
  char* buff = NULL;
  size_t totalSize = 0;
  NetMessage *tcpMessage = (NetMessage*) NULL;
  int resultSelect;
  fd_set fds;
  fd_set fds_err;
  size_t i;
  TcpSocket *socket = NULL;
  TcpSocket *lastSocket = NULL;
  int highestSocketId = 0;
  
  FD_ZERO(&fds);
  FD_ZERO(&fds_err);

  for (i = 0; i<sockets.size(); ++i) {
    socket = sockets[i];
    
    if (socket != NULL) {
      FD_SET(socket->socketId,&fds);
      FD_SET(socket->socketId,&fds_err);
      if (highestSocketId < socket->socketId) highestSocketId = socket->socketId;
    }
  }
  
  while (tcpMessage == NULL) {
    resultSelect = pselect(highestSocketId+1, &fds, NULL, &fds_err, NULL, NULL);
    
    if (resultSelect == -1) {
      if (socket_index != NULL) {
        for (i = 0; i<sockets.size(); ++i) {
          if ((sockets[i] != NULL) && (FD_ISSET(sockets[i]->socketId, &fds_err))) {
            *socket_index = i;
            break;
          }
        }
      }
      launchNetExceptionForSelect(errno);
    } else {
      for (i = 0; i<sockets.size(); ++i) {
        if ((sockets[i] != NULL) && (FD_ISSET(sockets[i]->socketId, &fds))) {
          socket = sockets[i];
          break;
        }
      }
      if (socket != NULL) {
        // Save resiliant data if data is notified on a different socket than last one
        if ((lastSocket != NULL) && (lastSocket != socket)) {
          lastSocket->resiliantDataSize = totalSize;
          lastSocket->resiliantData = (char*) malloc(lastSocket->resiliantDataSize*sizeof(char));
          memcpy(lastSocket->resiliantData, buff,lastSocket->resiliantDataSize);
          lastSocket = socket;
        }
        if (socket_index != NULL) {
          *socket_index = i;
        }
        Logger::log(INFO) << "Data available on socket " << i << " !" << Logger::endm();
        socket->receiveData(buff, totalSize);
        tcpMessage = socket->tryBuildObject(buff,totalSize);
      } else {
        throw (NetException::NetException(SOCKET_ID_WRONG,"Socket id not known."));
      }
    }
  }

  SerializationManager *serManager =
    SerializationManager::getSerializationManager();
  void *object = serManager->deserialize(*tcpMessage, false);
   
  free(buff);
  delete tcpMessage;
  return (Serializable*) object;
}

Serializable *TcpSocket::receiveObject(const std::vector<TcpSocket*> &sockets,
  uint64_t nanosec, int *socket_index) {
  
  time_t sec = nanosec / NB_USEC_PER_SEC;
  long nsec = nanosec % NB_USEC_PER_SEC;
  return receiveObject(sockets, sec,nsec, socket_index);
}

Serializable *TcpSocket::receiveObject(const std::vector<TcpSocket*> &sockets, 
  time_t sec, long nanosec, int *socket_index) {
  
  char* buff = NULL;
  size_t totalSize = 0;
  NetMessage *tcpMessage = (NetMessage*) NULL;
  int resultSelect;
  fd_set fds;
  size_t i;
  TcpSocket *socket = NULL;
  TcpSocket *lastSocket = NULL;
  int highestSocketId = 0;
  timespec timeout; 
  Timer timer;
  time_t sec_elasped;
  long nanosec_elapsed;
  bool again = true;

  timeout.tv_sec = sec; 
  timeout.tv_nsec = nanosec;  

  timer.startTimer();

  FD_ZERO(&fds);

  for (i = 0; i<sockets.size(); ++i) {
    socket = sockets[i];
    
    if (socket != NULL) {
      FD_SET(socket->socketId,&fds);
      if (highestSocketId < socket->socketId) highestSocketId = socket->socketId;
    }
  }
  
  do {

    resultSelect = pselect(highestSocketId+1, &fds, NULL, NULL, &timeout, NULL);
    
    if (resultSelect == -1) {
      launchNetExceptionForSelect(errno);
    } else if (resultSelect == 0) {
      if (buff != NULL) {
        lastSocket->resiliantDataSize = totalSize; 
        lastSocket->resiliantData = (char*) malloc(lastSocket->resiliantDataSize*sizeof(char));
        memcpy(lastSocket->resiliantData, buff,lastSocket->resiliantDataSize);
      }
      throw (NetException::NetException(SOCKET_TIMEOUT,"Timeout on receive polling occurs."));
    } else {
      for (i = 0; i<sockets.size(); ++i) {
        if ((sockets[i] != NULL) && (FD_ISSET(sockets[i]->socketId, &fds))) {
          socket = sockets[i];
          break;
        }
      }
      if (socket != NULL) {
        // Save resiliant data if data is notified on a different socket than last one
        if ((lastSocket != NULL) && (lastSocket != socket)) {
          lastSocket->resiliantDataSize = totalSize;
          lastSocket->resiliantData = (char*) malloc(lastSocket->resiliantDataSize*sizeof(char));
          memcpy(lastSocket->resiliantData, buff,lastSocket->resiliantDataSize);
          lastSocket = socket;
        }
        
        if (socket_index != NULL) {
          *socket_index = i;
        }
        socket->receiveData(buff, totalSize);
        tcpMessage = socket->tryBuildObject(buff,totalSize);
        
        if (tcpMessage == NULL) {
          timer.getDiffTime(sec_elasped,nanosec_elapsed,true);
          socket->computeTimeDiff(sec,nanosec, sec_elasped, nanosec_elapsed,
            timeout.tv_sec, timeout.tv_nsec);
        } else {
          again = false;
        }
      } else {
        throw (NetException::NetException(SOCKET_ID_WRONG,"Socket id not known."));
      }
    }
  } while (again);
    
  SerializationManager *serManager =
    SerializationManager::getSerializationManager();
  void *object = serManager->deserialize(*tcpMessage, false);
   
  free(buff);
  delete tcpMessage;
  return (Serializable*) object;
}

std::string *TcpSocket::receiveString(void) {
  char* buff = NULL;
  size_t totalSize = 0;
  std::string *string = NULL;

  while (string == NULL) {
    receiveData(buff, totalSize);
    string = tryBuildString(buff, totalSize);
  }
  free(buff);

  return string;
}

Buffer<char> *TcpSocket::receiveBytes(void) {
  Buffer<char> *buff = new Buffer<char>();
  if (resiliantData != NULL) {
    buff->copyIn(0,resiliantData,resiliantDataSize);
    free(resiliantData);
    resiliantData = NULL;
    resiliantDataSize = 0;
  } else {
    char receiveBuff[bufferSize];
    ssize_t sizeReceived = 
      recv(socketId, &receiveBuff, bufferSize, 0);
    if (sizeReceived == -1) {
      delete buff;
      launchNetExceptionForSendReceive(errno);
    } else if (sizeReceived == 0) {
      delete buff;
      throw NetSocket::NetException(SOCKET_CLOSED, 
        "Distant socket has been closed.");
    } else {
      buff->copyIn(buff->size(), receiveBuff, sizeReceived);
    }
  }
  return buff;
}

NetAddress TcpSocket::getDistantAddress() const {
  struct sockaddr_in distAddr;
  socklen_t size;
  
  size = sizeof(distAddr);
  if (getpeername(socketId, (sockaddr*) &distAddr, &size) == -1) {
    launchNetExceptionForGetPeerName(errno);
  }
  
  return NetAddress(distAddr);
}

void TcpSocket::launchNetExceptionForGetPeerName(int code) {
  std::string errorMessage;
  switch (code) {
    case EINVAL :
      errorMessage = "namelen is invalid (e.g., is negative).";
      break;
    case EBADF :
      errorMessage = "d is not a valid descriptor";
      break;
    case EFAULT :
      errorMessage =  "The name parameter points to memory not in a valid part "
                      "of the process address space ";
      break;
    case ENOBUFS :
      errorMessage =  "Insufficient resources were available in the system to p"
                      "erform the operation.";

    case ENOTCONN:
      errorMessage =  "The socket is not connected.";

      break;
    case ENOTSOCK :
      errorMessage =  "The argument s is a file, not a socket.";
      
    default :
      errorMessage = "Unknown error";
      break;
  }
  throw (NetAddress::NetException(errno,errorMessage));
}

#include <iostream>
void TcpSocket::launchNetExceptionForConnect(int code) {
  std::string errorMessage;
  switch (code) {
    case EACCES : 
      errorMessage =  "Write permission is denied on the socket file, or searc"
                      "h permission is denied for one of the directories in th"
                      "e path prefix.";
      break;
    //case EACCES :
    case EPERM :
      errorMessage =  "The user tried to connect to a broadcast address withou"
                      "t having the socket broadcast flag enabled or the conne"
                      "ction request failed because of a local firewall rule.";
      break;
    case EADDRINUSE :
      errorMessage =  "Local address is already in use.";
      break;
    case EAFNOSUPPORT :
      errorMessage =  "The passed address didn't have the correct address fami"
                      "ly in its sa_family field.";
      break;
    case EAGAIN :
      errorMessage =  "No more free local ports or insufficient entries in the"
                      " routing cache.";
      break;
    case EALREADY :
      errorMessage =  "The socket is non-blocking and a previous connection at"
                      "tempt has not yet been completed.";
      break;
    case EBADF : 
      errorMessage =  "The file descriptor is not a valid index in the descrip"
                      "tor table.";

      break;
    case ECONNREFUSED :
      errorMessage =  "No-one listening on the remote address.";
      break;
    case EFAULT :
      errorMessage =  "The socket structure address is outside the user's addr"
                      "ess space.";
      break;
    case EINPROGRESS:
      errorMessage =  "The socket is non-blocking and the connection cannot be"
                      " completed immediately.";
      break;
    case EINTR :
      errorMessage =  "The system call was interrupted by a signal that was ca"
                      "ught.";
      break;
    case EISCONN:
      errorMessage =  "The socket is already connected.";
      break;
    case ENETUNREACH:
      errorMessage =  "Network is unreachable.";
      break;
    case ENOTSOCK:
      errorMessage =  "The file descriptor is not associated with a socket.";
      break;
    case ETIMEDOUT:
      errorMessage =  "Timeout while attempting connection.";
      break;
    default:
      errorMessage =  "Unknown message";  
  }
  throw (NetSocket::NetException(errno,errorMessage));
}

TcpServerSocket::TcpServerSocket(): NetSocket(SOCK_STREAM) {
  listenTo(DEFAULT_BACKLOG); 
}

TcpServerSocket::TcpServerSocket(int localPort): NetSocket(SOCK_STREAM) {
  BooleanOption opt(BooleanOption::reuseAddrOpt, true);
  setSocketOption(opt);
  bindSocket(localPort);
  listenTo(DEFAULT_BACKLOG);
}

TcpServerSocket::TcpServerSocket(int localPort, int backlog): NetSocket(SOCK_STREAM) {
  BooleanOption opt(BooleanOption::reuseAddrOpt, true);
  setSocketOption(opt);
  bindSocket(localPort);
  listenTo(backlog);
}

void TcpServerSocket::listenTo(int backlog) {
  int result;

  result = listen(socketId, backlog);
  if (result == -1) {
    launchNetExceptionForListen(errno);
  }
}

TcpSocket *TcpServerSocket::acceptConnection() {
  int result;

  result = accept(socketId, NULL, NULL);
  if (result == -1) {
    launchNetExceptionForAccept(errno);
  }
  return new TcpSocket(result); 
}

TcpSocket *TcpServerSocket::acceptConnection(uint64_t nanosec) {
  time_t sec = nanosec / NB_USEC_PER_SEC;
  long nsec = nanosec % NB_USEC_PER_SEC;
  return acceptConnection(sec,nsec);
}

TcpSocket *TcpServerSocket::acceptConnection(time_t sec, long nanosec) {
  int resultSelect;
  timespec timeout; 
  fd_set fds;

  timeout.tv_sec = sec; 
  timeout.tv_nsec = nanosec;  

  FD_ZERO(&fds);
  FD_SET(socketId,&fds);
    
  resultSelect = pselect(socketId+1, &fds, NULL, NULL, &timeout,NULL);
  if (resultSelect == -1) {
    launchNetExceptionForSelect(errno);
  } else if (resultSelect == 0) {
    throw (NetException::NetException(SOCKET_TIMEOUT,"Timeout on accept occurs."));
  } else {
    return acceptConnection();
  }
  return NULL;
}

void TcpServerSocket::launchNetExceptionForListen(int code) {
  std::string errorMessage;
  switch (code) {
    case EADDRINUSE :
      errorMessage =  "Another socket is already listening on the same port.";
      break;
    case EBADF :
      errorMessage =  "The argument sockfd is not a valid descriptor.";
      break;
    case ENOTSOCK:
      errorMessage =  "The argument sockfd is not a socket.";
      break;
    case EOPNOTSUPP:
      errorMessage =  "The socket is not of a type that supports the listen op"
                      "eration.";
      break;
    default :
      errorMessage = "Unknown error";
      break;
  }
  throw (NetAddress::NetException(errno,errorMessage));
}

void TcpServerSocket::launchNetExceptionForAccept(int code) {
  std::string errorMessage;
  switch (code) {
    case EAGAIN:
    //case EWOULDBLOCK:
      errorMessage =  "The socket is marked non-blocking and no connections ar"
                      "e present to be accepted.";
      break;
    case EBADF :
      errorMessage =  "The descriptor is invalid.";
      break;
    case ECONNABORTED:
      errorMessage =  "A connection has been aborted.";
      break;
    case EINTR:
      errorMessage =  "The system call was interrupted by a signal that was ca"
                      "ught before a valid connection arrived.";
      break;
    case EINVAL:
      errorMessage =  "Socket is not listening for connections, or addrlen is "
                      "invalid (e.g., is negative).";
      break;
    case EMFILE:
      errorMessage =  "The per-process limit of open file descriptors has been"
                      " reached.";
      break;
    case ENFILE:
      errorMessage =  "The system limit on the total number of open files has "
                      "been reached.";
      break;
    case ENOTSOCK:
      errorMessage =  "The descriptor references a file, not a socket.";
      break;
    case EOPNOTSUPP:
      errorMessage =  "The referenced socket is not of type SOCK_STREAM.";
      break;
    case EFAULT:
      errorMessage =  "The addr argument is not in a writable part of the user"
                      " address space.";
      break;
    case ENOBUFS:
    case ENOMEM:
      errorMessage =  "Not enough free memory.  This often means that the memo"
                      "ry allocation is limited by the socket buffer limits, n"
                      "ot by the system memory.";
      break;
    case EPROTO:
      errorMessage =  "Protocol error.";
      break;
#ifdef __LINUX
    case EPERM:
      errorMessage =  "Firewall rules forbid connection.";
      break;
#endif
    default :
      errorMessage = "Unknown error";
      break;
  }
  throw (NetAddress::NetException(errno,errorMessage));
}
