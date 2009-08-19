#include <sys/socket.h>  // socket()
#include <netdb.h>       // gethostbyname(), htons()
#include <string.h> 	// memcpy
#include <stdlib.h> //free
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>

#include "udp_socket.h"
#include "serialization_manager.h"
#include "types_utils.h"


NetSocket::NetSocket(void) {}

NetSocket::~NetSocket() {
}

int NetSocket::createSocket(int domain) {
  int socketId = socket (AF_INET, domain, 0) ;
  if (socketId == -1) {
    throw NetSocket::NetException(errno);
  }
  return socketId;
}


void NetSocket::bindSocket(int port) {
  struct sockaddr_in serverAddr;
  
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(port);
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  
  int status = bind (getSocketId(), (sockaddr*) &serverAddr, sizeof(serverAddr));
  if (status == -1) {
    throw NetSocket::NetException(errno); 
  }
}

NetAddress NetSocket::getLocalAddress() const {
  struct sockaddr_in localAddr;
  socklen_t size;
  NetAddress address;
  
  size = sizeof(localAddr);
  if (getsockname(getSocketId(), (sockaddr*) &localAddr, &size) == -1) {
    throw NetSocket::NetException(errno);
  }

  address.setAddress(NetAddress::getLocalIp());
  address.setPort((int)ntohs(localAddr.sin_port));
  
  return address;
}

NetAddress NetSocket::getLocalAddress(const char *interface) const {
  std::map<std::string, NetAddress> *interfaces = getLocalAddresses();
  std::map<std::string, NetAddress>::iterator iter;

  iter = interfaces->find(interface);
  if (iter == interfaces->end()) {
    delete interfaces;
    throw (NetSocket::NetException(HOST_NOT_FOUND, "Interface not found"));
  } else {
    NetAddress address = iter->second;
    delete interfaces;
    return address;
  }
}

std::map<std::string,NetAddress> *NetSocket::getLocalAddresses() const {
  char buf[1024];
  char addressBuf[1024];
  struct ifconf ifc;
  struct ifreq *ifr;
  int nInterfaces;
  std::map<std::string,NetAddress> *returnMap;
 
  ifc.ifc_len = sizeof(buf);
  ifc.ifc_buf = buf;
  if(ioctl(getSocketId(), SIOCGIFCONF, &ifc) == -1){
    throw NetSocket::NetException(errno);
  }
  
  returnMap = new std::map<std::string, NetAddress>();
  ifr = ifc.ifc_req;
  nInterfaces = ifc.ifc_len / sizeof(struct ifreq);
  
  for(int i = 0; i < nInterfaces; ++i) {
    struct ifreq *item = &ifr[i];
    struct sockaddr_in *addr = (sockaddr_in*)&(item->ifr_addr);

    inet_ntop(AF_INET,(void*)&(addr->sin_addr),addressBuf,sizeof(addressBuf));

    (*returnMap)[item->ifr_name] = NetAddress(addressBuf,
      getLocalAddress().getPort());
  }
  return returnMap;
}

NetSocket::NetException::NetException(int code)
  : Exception(code) {
}

NetSocket::NetException::NetException(int code, std::string message)
  : Exception(code, message) {
}

NetSocket::Option::Option(int name, socklen_t len) {
  this->name = name;
  this->len = len;
}

NetSocket::Option::~Option(void) {
}

int NetSocket::Option::getName(void) {
  return name;
}

socklen_t *NetSocket::Option::getSize(void) {
  return &len;
}

NetSocket::IntOption::IntOption(int name, int value): 
  NetSocket::Option(name, sizeof(value)) {
  this->value = value;
}

NetSocket::IntOption::~IntOption(void) {
}

void *NetSocket::IntOption::getValue(void) {
  return (void *) &value; 
}

const int NetSocket::IntegerOption::names[] = 
  {SO_SNDBUF, SO_RCVBUF, SO_SNDLOWAT, SO_RCVLOWAT, SO_ERROR};

NetSocket::IntegerOption::IntegerOption(IntegerOption::Name name, int value)
  : NetSocket::IntOption(names[name], value) {
}

NetSocket::IntegerOption::IntegerOption(IntegerOption::Name name)
  : NetSocket::IntOption(names[name], -1) {
}

NetSocket::IntegerOption::~IntegerOption(void) {
}

int NetSocket::IntegerOption::getIntegerValue() {
  return value;
}

const int NetSocket::BooleanOption::names[] = 
  {SO_DEBUG, SO_BROADCAST, SO_REUSEADDR, SO_KEEPALIVE, SO_OOBINLINE, SO_DONTROUTE};

NetSocket::BooleanOption::BooleanOption(BooleanOption::Name name, bool value) 
  : NetSocket::IntOption(names[name], (value) ? 1 : 0) {
}

NetSocket::BooleanOption::BooleanOption(BooleanOption::Name name)
  : NetSocket::IntOption(names[name], 0) {
}

NetSocket::BooleanOption::~BooleanOption(void) {
}

bool NetSocket::BooleanOption::getBooleanValue() {
  return (value != 0);
}

void NetSocket::setSocketOption(NetSocket::Option &option) {
  int result;

  result = setsockopt(getSocketId(), SOL_SOCKET, option.getName(),
      option.getValue(), *(option.getSize()));

  if (result == -1) {
    throw NetSocket::NetException(errno);
  }
}

void NetSocket::getSocketOption(NetSocket::Option *option) {
  int result;

  result = getsockopt(getSocketId(), SOL_SOCKET, option->getName(),
      option->getValue(), option->getSize());

  if (result == -1) {
    throw NetSocket::NetException(errno);
  }
}



int IONetSocket::getSocketId(void) const {
  return fd;
}

IONetSocket::IONetSocket(void) {
}

IONetSocket::IONetSocket(int domain) {
  fd = createSocket(domain);
}

IONetSocket::IONetSocket(int domain, const NetAddress &address) {
  fd = createSocket(domain);
  connectSocket(address);  
}

void IONetSocket::connectSocket(const NetAddress &address) {
  struct sockaddr_in distAddr;
  int result;
  
  address.getSockAddr(&distAddr);
  result = connect(fd, (sockaddr*)&distAddr, sizeof(distAddr));
  if (result == -1) {
    throw NetSocket::NetException(errno); 
  }
}

void IONetSocket::connectSocket(const NetAddress &address, uint64_t nanosec) {
  time_t sec;
  long nsec;

  nanosecToSecNsec(nanosec, &sec, &nsec);
  connectSocket(address, sec, nsec);
}

void IONetSocket::connectSocket(const NetAddress &address, time_t sec, long nanosec) {
  int result; 
  long flags;
  
  // Set socket in O_NONBLOCK
  flags = fcntl(fd, F_GETFL, NULL);
  result = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
  if (result == -1) {
    throw NetSocket::NetException(errno);
  }

  try {
    connectSocket(address);
  } catch (Exception &e) {
    if (e.getCode() != EINPROGRESS) {
      throw e;
    } else {
      StreamWFRResult waitResult = waitForReady(STREAM_WFR_WRITE, sec, nanosec);
      //Timeout or error
      if ((waitResult.setIsNone()) || (waitResult.setIsError())) {
        throw waitResult.e;
      } else {
        
        IntegerOption opt(NetSocket::IntegerOption::error);
        getSocketOption(&opt);
        
        // Set socket to initial state
        result = fcntl(fd, F_SETFL, flags);
        if (result == -1) {
          throw NetSocket::NetException(errno);
        }

        if (opt.getIntegerValue() != 0) {
          throw NetSocket::NetException(opt.getIntegerValue());
        }
      }
    }
  }
}

NetAddress IONetSocket::getDistantAddress() const {
  struct sockaddr_in distAddr;
  socklen_t size;

  size = sizeof(distAddr);
  if (getpeername(fd, (sockaddr*) &distAddr, &size) == -1) {
    throw NetSocket::NetException(errno);
  }

  return NetAddress(distAddr);
}

