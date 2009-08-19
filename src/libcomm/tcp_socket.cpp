#include <sys/types.h>
#include <sys/socket.h>  
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h> //free
#include <string.h>

#include "tcp_socket.h"
#include "serialization_manager.h"
#include "timer.h"

#define DEFAULT_BACKLOG 15

const int MAX_IOV = sysconf(_SC_IOV_MAX);

TcpSocket::TcpSocket(int socketId): IONetSocket() {
  fd = socketId;
}

ssize_t TcpSocket::readRawData(   char *buffer, size_t size, int flags,
                                  NetAddress *addr) {
  ssize_t bytesRead;
  bytesRead = recv(fd, buffer, size, flags);
 
  switch (bytesRead) {
    case -1:
      throw InputStream::InputStreamException(errno);
    case 0:
      throw InputStream::InputStreamException(EX_STREAM_CLOSED, "Stream has been closed.");
    default:
      return bytesRead;
  }
}

/*ssize_t TcpSocket::peekData(  char *buffer, size_t size,
                              NetAddress *addr) {
  ssize_t bytesRead;
  bytesRead = recv(fd, buffer, size, MSG_PEEK);

  switch (bytesRead) {
    case -1:
      throw InputStream::InputStreamException(errno);
    case 0:
      throw InputStream::InputStreamException(EX_STREAM_CLOSED, "Stream has been closed.");
    default:
      return bytesRead;
  }
}*/

ssize_t TcpSocket::writeData( const char *data, size_t size, int flags,
                              const NetAddress *addr) {
  ssize_t bytesWritten;
  bytesWritten = send(fd, data, size, flags);

  if (bytesWritten == -1) {
    throw OutputStream::OutputStreamException(errno, size);
  }

  return bytesWritten;
}
ssize_t TcpSocket::writeData( const struct iovec *iov, int iovcnt,
                              const NetAddress *addr) {
  int currentIovCnt;
  int totalIovCnt = 0;
  size_t totalQuantityWritten = 0;
  ssize_t quantityWritten = 0;

  while (totalIovCnt < iovcnt) {
    currentIovCnt = iovcnt - totalIovCnt;
    if (currentIovCnt > MAX_IOV) currentIovCnt = MAX_IOV;

    quantityWritten = writev(fd, &(iov[totalIovCnt]), currentIovCnt);
    if (quantityWritten == -1) {
      size_t toWrite;
      char *dataLeft;
      
      dataLeft = generateRemainingData(iov,iovcnt, totalQuantityWritten, &toWrite);
      throw OutputStream::OutputStreamException(errno, toWrite,  dataLeft, quantityWritten);
    }
    totalQuantityWritten += quantityWritten;
    totalIovCnt += currentIovCnt;
  }
  
  return totalQuantityWritten;
}

TcpSocket::TcpSocket(): IONetSocket(SOCK_STREAM) {
}

TcpSocket::TcpSocket(const NetAddress &address): IONetSocket(SOCK_STREAM, address) {
}

TcpSocket::~TcpSocket() {
}

void TcpSocket::shutdownSocket(bool read, bool write) {
  int result;
  int how = ((read) ? ((write) ? SHUT_RDWR : SHUT_RD) : (SHUT_WR));

  result = shutdown(fd, how);
  if (result == -1) {
    throw NetSocket::NetException(errno);
  }
}

TcpServerSocket::TcpServerSocket() {
  socketId = createSocket(SOCK_STREAM);
  listenTo(DEFAULT_BACKLOG); 
}

TcpServerSocket::TcpServerSocket(int localPort) {
  socketId = createSocket(SOCK_STREAM);
  BooleanOption opt(BooleanOption::reuseAddrOpt, true);
  setSocketOption(opt);
  bindSocket(localPort);
  listenTo(DEFAULT_BACKLOG);
}

TcpServerSocket::TcpServerSocket(int localPort, int backlog) {
  createSocket(SOCK_STREAM);
  BooleanOption opt(BooleanOption::reuseAddrOpt, true);
  setSocketOption(opt);
  bindSocket(localPort);
  listenTo(backlog);
}

void TcpServerSocket::listenTo(int backlog) {
  int result;

  result = listen(socketId, backlog);
  if (result == -1) {
    throw NetSocket::NetException(errno);
  }
}

int TcpServerSocket::getSocketId(void) const {
  return socketId;
}

TcpSocket *TcpServerSocket::acceptConnection() {
  int result;

  result = accept(socketId, NULL, NULL);
  if (result == -1) {
    throw NetSocket::NetException(errno);
  }
  return new TcpSocket(result); 
}

TcpSocket *TcpServerSocket::acceptConnection(uint64_t nanosec) {
  time_t sec;
  long nsec;

  nanosecToSecNsec(nanosec, &sec, &nsec);
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
    throw NetSocket::NetException(errno);
  } else if (resultSelect == 0) {
    throw (NetException::NetException(EX_ACCEPT_TIMEOUT,"Timeout on accept occurs."));
  } else {
    return acceptConnection();
  }
  return NULL;
}

void TcpServerSocket::closeServer(void) {
  int result;
  
  result = close(socketId);
  if (result == -1) {
    throw NetSocket::NetException(errno); 
  }
}
