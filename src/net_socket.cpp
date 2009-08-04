#include <sys/socket.h>  // socket()
#include <netdb.h>       // gethostbyname(), htons()
#include <string.h> 	// memcpy
#include <stdlib.h> //free
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include <iostream>

#include "udp_socket.h"
#include "serialization_manager.h"


//TODO Add security when wrong udp packets!

#define bufferSize 1500
#define NB_USEC_PER_SEC 1000000000

NetSocket::NetSocket() {
}

NetSocket::NetSocket(int domaine) {
  socketId = socket (AF_INET, domaine, 0) ;
  if (socketId == -1) {
    launchNetExceptionForSocket(errno);
  }
}

NetSocket::~NetSocket() {
}

void NetSocket::launchNetExceptionForSocket(int code) {
  std::string errorMessage;
  switch (code) {
    case EACCES : 
      errorMessage = "Permission for creating this socket denied.";
      break;
    case EAFNOSUPPORT :
      errorMessage = "Address familiy not supported (Should never happen!)";
      break;
    case EINVAL :
      errorMessage = "Unknown protocol or address family (Should never happen)";
      break;
    /*case EMFILEA :
      errorMessage = "File table full.";
      break;*/
    case ENFILE :
      errorMessage = "Maximum number of files opened reached.";
      break;
    case ENOBUFS :
    case ENOMEM :
      errorMessage = "Insufficient space for allocating buffers.";
      break;
    case EPROTONOSUPPORT :
      errorMessage = "Protocol (type) is not available in this communication d";
      errorMessage += "omain (Should never happen!).";
      break;
    default :
      errorMessage = "Unknown error";
      break;
  }
  throw (NetSocket::NetException(errno,errorMessage));
}

void NetSocket::bindSocket(int port) {
  struct sockaddr_in serverAddr;
  
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(port);
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  
  int status = bind (socketId, (sockaddr*) &serverAddr, sizeof(serverAddr));
  if (status == -1) {
    std::string errorMessage;
    switch (errno) {
      case EACCES : 
        errorMessage = "Protected address. Permission denied.";
        break;
      case EADDRINUSE :
        errorMessage = "Address already in use.";
        break;
      case EBADF :
        errorMessage = "socketId not valid. (Should never happen!)";
        break;
      case EINVAL :
        errorMessage = " The socket is already bound to an address.";
        break;
      case ENOTSOCK :
        errorMessage = "socketId is a file descriptor, not a socket descriptor";
        errorMessage += " (Should never happen!)";
        break;
      /*
      case EADDRNOTAVAIL :
        errorMessage = "Interface doesn't exist, or non-local address given. ";
        errorMessage += "(Should never happen!)";
        break;
      case EFAULT :
        errorMessage = "Address pointer non valid.";
        break;
      case ELOOP :
        errorMessage = "Too many symbolic links were encountered in resolving addr.";
        break;
      case ENAMETOOLONG :
        errorMessage = "addr is too long (Should never happen!)";
        break;
      case ENOENT :
        errorMessage = "The file does not exist (Should never happen!).";
        break;
      case ENOMEM : 
        errorMessage = "Insufficient kernel memory was available.";
        break;
      case ENOTDIR :
        errorMessage = "A component of the path prefix is not a directory.";
        break;
      case EROFS : 
        errorMessage = "The socket inode would reside on a read-only file system.";
        break;
      */
    default :
      errorMessage = "Unknown error";
      break;
    }
    throw (NetSocket::NetException(errno,errorMessage));
  }
}


void NetSocket::computeTimeDiff(time_t secf, long nanosecf, time_t secs, long nanosecs,
  time_t &secr, long nanosecr) {
  
  if ((nanosecs > nanosecf) && (secf > secs)) {
    secr = secf - secs - 1;
    nanosecr = nanosecf + (NB_NSEC_IN_SEC - nanosecs);
  } else if (secf >= secs) {
    secr = secf - secs;
    nanosecr = nanosecf - nanosecs;
  } else {
    secr = 0;
    nanosecr = 0;
  }
}

void NetSocket::closeSocket() {
  close(socketId);
}

void NetSocket::setSocketOption(NetSocket::Option &option) {
  int result;
  
  result = setsockopt(socketId, SOL_SOCKET, option.getName(),
    option.getValue(), *(option.getSize()));

  if (result == -1) {
    launchNetExceptionForGetSetSockOpt(errno);
  }
}

void NetSocket::getSocketOption(NetSocket::Option *option) {
  int result;

  result = getsockopt(socketId, SOL_SOCKET, option->getName(),
    option->getValue(), option->getSize());

  if (result == -1) {
    launchNetExceptionForGetSetSockOpt(errno);
  }
}

void NetSocket::launchNetExceptionForGetSetSockOpt(int code) {
  std::string errorMessage;
  switch (code) {
    case EBADF:
      errorMessage =  "The argument s is not a valid descriptor.";
      break;
    case EFAULT:
      errorMessage =  "The address pointed to by optval is not in a valid par"
                      "t of the process address space. For getsockopt(), this"
                      " error may also be returned if optlen is not in a vali"
                      "d part of the process address space.";
      break;
    case EINVAL:
      errorMessage =  "optlen invalid in setsockopt().";
      break;
    case ENOPROTOOPT:
      errorMessage =  "The option is unknown at the level indicated.";
      break;
    case ENOTSOCK:
      errorMessage =  "The argument s is a file, not a socket.";
    default :
      errorMessage =  "Unknown error";
  }
  throw (NetSocket::NetException(errno,errorMessage));
}


NetAddress NetSocket::getLocalAddress() const {
  struct sockaddr_in localAddr;
  socklen_t size;
  NetAddress address;
  
  size = sizeof(localAddr);
  if (getsockname(socketId, (sockaddr*) &localAddr, &size) == -1) {
    launchNetExceptionForGetSockName(errno);
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
  if(ioctl(socketId, SIOCGIFCONF, &ifc) == -1){
    launchNetExceptionForIoctl(errno);
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

void NetSocket::launchNetExceptionForSelect(int code) {
  std::string errorMessage;
  switch (code) {
    case EBADF :
      errorMessage =  "An invalid file descriptor was given in one of the sets"
                      ".  (Perhaps a file descriptor that was already closed, "
                      "or one on which an error has occurred.)";
      break;
    case EINTR:
      errorMessage =  "A signal was caught";
      break;
    case EINVAL:
      errorMessage =  "nfds is negative or the value contained within timeout i"
                      "s invalid.";
      break;
    case ENOMEM:
      errorMessage =  "unable to allocate memory for internal tables.";
      break;
    default:
      errorMessage =  "Unknown error";
      break;
  }
  throw (NetSocket::NetException(errno,errorMessage));
}

void NetSocket::launchNetExceptionForSendReceive(int code) {
  std::string errorMessage;
  switch (code) {
    case EAGAIN :
      errorMessage = "The socket is marked non-blocking and the receive operat";
      errorMessage += "ion would block, or a receive timeout had been set and ";
      errorMessage += "the timeout expired before data was received.";
      break;
    case EBADF :
      errorMessage = "The argument s is an invalid descriptor (Should never ha";
      errorMessage += "ppen!)";
      break;
    case ECONNREFUSED :
      errorMessage = "A remote host refused to allow the network connection ";
      errorMessage += "(typically because it is not running the requested serv";
      errorMessage += "ice).";
      break;
    case EFAULT :
      errorMessage = "The receive buffer pointer(s) point outside the process'";
      errorMessage += "s address  space (Should never happen!)";
      break;
    case EINTR : 
      errorMessage = "The receive was interrupted by delivery of a signal befo";
      errorMessage += "re any data were available.";
      break;
    case EINVAL : 
      errorMessage = "Invalid argument passed (Should never happen!)";
      break;
    case ENOMEM :
      errorMessage = "Could not allocate memory for the call.";
      break;
    case ENOTCONN :
      errorMessage = "The socket is associated with a connection-oriented prot";
      errorMessage += "ocol and has not been connected (Should never happen!).";
      break;
    case ENOTSOCK : 
      errorMessage = "The argument s does not refer to a socket (Should never ";
      errorMessage += "happen!)";
      break;
    case ECONNRESET :
      errorMessage = "Connection reset by peer.";
      break;
    case EDESTADDRREQ :
      errorMessage = "The socket is not connection-mode, and no peer address i";
      errorMessage += "s set.";
      break;
    case EISCONN :
      errorMessage = "The connection-mode socket was connected already but a r";
      errorMessage += "ecipient was specified. (Now either this error is retur";
      errorMessage += "ned, or the recipient specification is ignored.)";
      break;
    case EMSGSIZE :
      errorMessage = "The socket type requires that message be sent atomically";
      errorMessage += ", and the size of the message to be sent made this impo";
      errorMessage += "ssible.";
      break;
    case ENOBUFS :
      errorMessage = "The output queue for a network interface was full.";
      break;
    case EOPNOTSUPP :
      errorMessage = "Some bit in the flags argument is inappropriate for the ";
      errorMessage += "socket type (Should never happen!);";
      break;
    case EPIPE : 
      errorMessage = "The local end has been shut down on a connection oriente";
      errorMessage += "d socket. (Should never happen!).";
      break;
    default :
      errorMessage = "Unknown error";
      break;
  }
  throw (NetSocket::NetException(errno,errorMessage));
}

void NetSocket::launchNetExceptionForIoctl(int code) {
  std::string errorMessage;
  switch (code) {
    case EINVAL :
      errorMessage = "Request or argp is not valid.";
      break;
    case EBADF :
      errorMessage = "d is not a valid descriptor";
      break;
    case EFAULT :
      errorMessage = "argp references an inaccessible memory area.";
      break;
    case ENOTTY :
      errorMessage = "d is not associated with a character special device OR T";
      errorMessage += "he specified request does not apply to the kind of obje";
      errorMessage += "ct that the descriptor d references.";
      break;
    default :
      errorMessage = "Unknown error";
      break;
  }
  throw (NetAddress::NetException(errno,errorMessage));
}

void NetSocket::launchNetExceptionForGetSockName(int code) {
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
      break;
    case ENOTSOCK :
      errorMessage =  "The argument s is a file, not a socket.";
      
    default :
      errorMessage = "Unknown error";
      break;
  }
  throw (NetAddress::NetException(errno,errorMessage));
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
  {SO_SNDBUF, SO_RCVBUF, SO_SNDLOWAT, SO_RCVLOWAT};

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
