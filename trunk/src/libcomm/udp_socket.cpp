#include <sys/socket.h>  // socket()
#include <netdb.h>       // gethostbyname(), htons()
#include <string.h> 	// memcpy
#include <stdlib.h> //free
#include <netinet/in.h>  // sockaddr_in
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>

#include <iostream>

#include "udp_socket.h"
#include "serialization_manager.h"

#define MAX_UDP_PACKET_SIZE 1500
#define NB_USEC_PER_SEC 1000000000

const int MAX_IOV = sysconf(_SC_IOV_MAX);

UdpSocket::UdpSocket(): IONetSocket(SOCK_DGRAM), maxSize(MAX_UDP_PACKET_SIZE) {}

UdpSocket::UdpSocket(int localPort): IONetSocket(SOCK_DGRAM), maxSize(MAX_UDP_PACKET_SIZE) {
  bindSocket(localPort);
}

void UdpSocket::setMaximumSize(size_t maxSize) {
  this->maxSize = maxSize;
}

size_t UdpSocket::getMaximumSize(void) {
  return maxSize;
}

ssize_t UdpSocket::readRawData(   char *buffer, size_t size, int flags,
                                  NetAddress *addr) {
  struct sockaddr_in clientAddr;
  socklen_t sizeAddr = sizeof(clientAddr);
  ssize_t bytesRead;

  if (addr == NULL) {
    bytesRead = recv(fd, buffer, size, flags);
  } else {
    bytesRead = recvfrom(fd, buffer, size, flags, (sockaddr*) &clientAddr, &sizeAddr);
  }
 
  switch (bytesRead) {
    case -1:
      throw InputStream::InputStreamException(errno);
    case 0:
      throw InputStream::InputStreamException(EX_STREAM_CLOSED, "Stream has been closed.");
    default:
      if (addr != NULL) *addr = NetAddress(clientAddr);
      return bytesRead;
  }
}

/*ssize_t UdpSocket::peekData(  char *buffer, size_t size,
                              NetAddress *addr) {
  struct sockaddr_in clientAddr;
  socklen_t sizeAddr = sizeof(clientAddr);
  ssize_t bytesRead;
  if (addr == NULL) {
    bytesRead = recv(fd, buffer, size, MSG_PEEK);
  } else {
    bytesRead = recvfrom(fd, buffer, size, MSG_PEEK, (sockaddr*) &clientAddr, &sizeAddr);
  }

  switch (bytesRead) {
    case -1:
      throw InputStream::InputStreamException(errno);
    case 0:
      throw InputStream::InputStreamException(EX_STREAM_CLOSED, "Stream has been closed.");
    default:
      if (addr != NULL) *addr = NetAddress(clientAddr);
      return bytesRead;
  }
}*/

ssize_t UdpSocket::writeData( const char *data, size_t size, int flags,
                              const NetAddress *addr) {
  ssize_t bytesWritten;

  if ((maxSize != 0) && (size > maxSize)) {
    throw OutputStreamException(EX_OSTREAM_TOO_MUCH_DATA, size, 
      "Too much data to send into a single UDP packet.");
  }

  if (addr == NULL) {
    bytesWritten = send(fd, data, size, flags);
  } else {
    struct sockaddr_in clientAddr;
    socklen_t sizeAddr;

    addr->getSockAddr(&clientAddr);
    sizeAddr = sizeof(clientAddr);
    bytesWritten = sendto(fd, data, size, flags, (const sockaddr*) &clientAddr, sizeAddr);
  }


  if (bytesWritten == -1) {
    throw OutputStream::OutputStreamException(errno, size);
  }

  return bytesWritten;
}
ssize_t UdpSocket::writeData( const struct iovec *iov, int iovcnt,
                              const NetAddress *addr) {
  int currentIovCnt;
  size_t totalBytesToWrite = 0;
  ssize_t quantityWritten = 0;
  size_t totalQuantityWritten = 0;

  for (int i = 0; i<iovcnt; ++i) {
    totalBytesToWrite += iov[i].iov_len;
  }

  if ((maxSize != 0) && (totalBytesToWrite > maxSize)) {
    throw OutputStreamException(EX_OSTREAM_TOO_MUCH_DATA, totalBytesToWrite,
      "Too much data to send into a single UDP packet.");
  }

  if (addr == NULL) {
    int totalIovCnt = 0;

    while (totalIovCnt < iovcnt) {
      currentIovCnt = iovcnt - totalIovCnt;
      if (currentIovCnt > MAX_IOV) currentIovCnt = MAX_IOV;

      quantityWritten = writev(fd, &(iov[totalIovCnt]), currentIovCnt);
      if (quantityWritten == -1) {
        size_t toWrite;
        char *dataLeft;
        
        dataLeft = generateRemainingData(iov,iovcnt, totalQuantityWritten, &toWrite);
        throw OutputStream::OutputStreamException(errno, toWrite,  dataLeft,
          quantityWritten);
      }
      totalQuantityWritten += quantityWritten;
      totalIovCnt += currentIovCnt;
    }
  } else {
    struct sockaddr_in clientAddr;
    socklen_t sizeAddr;

    addr->getSockAddr(&clientAddr);
    sizeAddr = sizeof(clientAddr);

    for (int i = 0; i<iovcnt; ++i) {
      quantityWritten = sendto(fd, iov[i].iov_base, iov[i].iov_len, MSG_MORE,
        (const sockaddr*) &clientAddr, sizeAddr);

      if (quantityWritten == -1) {
        char *dataLeft;
        size_t toWrite;

        dataLeft = generateRemainingData(iov,iovcnt, totalQuantityWritten, &toWrite);
        throw OutputStream::OutputStreamException(errno, toWrite,  dataLeft, quantityWritten);
      }
      totalQuantityWritten += quantityWritten;
    }

    quantityWritten = sendto(fd, NULL, 0, 0,
      (const sockaddr*) &clientAddr, sizeAddr);
  }

  return totalQuantityWritten;
}

ssize_t UdpSocket::writeObject(const Serializable &object, const NetAddress &addr) {
  return writeObject2(object, &addr);
}

ssize_t UdpSocket::writeString(const std::string &string, const NetAddress &addr) {
  return writeString2(string, &addr);
}

ssize_t UdpSocket::writeBytes(const Buffer<char> &data, const NetAddress &addr, int flags) {
  return writeBytes2(data, flags, &addr);
}

Buffer<char> *UdpSocket::readBytes(Buffer<char> *buff, NetAddress *addr, int flags) {
  return readBytes2(buff, flags, addr);
}

Buffer<char> *UdpSocket::readBytes(Buffer<char> *buff, NetAddress *addr, uint64_t nanosec, int flags) {
  time_t sec;
  long nsec;

  nanosecToSecNsec(nanosec, &sec, &nsec);
  return readBytes(buff, addr, sec, nsec, flags);
}

Buffer<char> *UdpSocket::readBytes(Buffer<char> *buff, NetAddress *addr, time_t sec, long nanosec, int flags) {
  StreamWFRResult waitResult =
    waitForReady((StreamWFRSet) (STREAM_WFR_READ | STREAM_WFR_ERROR), sec, nanosec);
  // timeout || error
  if ((waitResult.setIsNone()) || (waitResult.setIsError())) {
    throw waitResult.e;;
  } else {
    return readBytes(buff, addr, flags);
  }
}

String *UdpSocket::readString(NetAddress *addr) {
  return readString2(addr);
}

String *UdpSocket::readString(NetAddress *addr, uint64_t nanosec) {
  time_t sec;
  long nsec;

  nanosecToSecNsec(nanosec, &sec, &nsec);
  return readString(addr, sec, nsec);
}

String *UdpSocket::readString(NetAddress *addr, time_t sec, long nanosec) {
  StreamWFRResult waitResult =
    waitForReady((StreamWFRSet) (STREAM_WFR_READ | STREAM_WFR_ERROR), sec, nanosec);
  // timeout || error
  if ((waitResult.setIsNone()) || (waitResult.setIsError())) {
    throw waitResult.e;;
  } else {
    return readString(addr);
  }
}

Serializable *UdpSocket::readObject(NetAddress *addr) {
  return readObject2(addr);
}

Serializable *UdpSocket::readObject(NetAddress *addr, uint64_t nanosec) {
  time_t sec;
  long nsec;

  nanosecToSecNsec(nanosec, &sec, &nsec);
  return readObject(addr, sec, nsec);
}

Serializable *UdpSocket::readObject(NetAddress *addr, time_t sec, long nanosec) {
  StreamWFRResult waitResult =
    waitForReady((StreamWFRSet) (STREAM_WFR_READ | STREAM_WFR_ERROR), sec, nanosec);
  // timeout || error
  if ((waitResult.setIsNone()) || (waitResult.setIsError())) {
    throw waitResult.e;;
  } else {
    return readObject(addr);
  }
}
