#include "net_address.h"

#include <netdb.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>


//#include "udp_socket.h"
NetAddress::NetAddress() : resolved(false) {}

NetAddress::NetAddress(std::string address, int port)
  : resolved(false) {
  this->address = address;
  this->port =  port;
}

NetAddress::NetAddress(const sockaddr_in &addr):
  resolved(false) {

  char addressBuf[1024];
  
  inet_ntop(AF_INET,((void*)&(addr.sin_addr)),
    addressBuf,sizeof(addressBuf));

  address = std::string(addressBuf); 
  port = ntohs(addr.sin_port);
}

std::string NetAddress::getAddress() const {
  return address;
}

std::string NetAddress::getCanonicalName() {
  if (!resolved) {
    resolve();
  }
  return hostname;
}

std::string NetAddress::getIp() {
  if (!resolved) {
    resolve();
  }
  return ip;
}

void NetAddress::resolve() {
  struct sockaddr_in sockAddr;

  getSockAddr(&sockAddr);
  hostname = NetAddress::findHostname(sockAddr);
  ip = NetAddress::findIp(address); 
  if (!resolved) resolved = true;
}

int NetAddress::getPort() const {
  return port;
}

void NetAddress::getSockAddr(sockaddr_in *saddr) const {
 struct hostent *hp = gethostbyname(address.c_str());
  if (hp == (struct hostent*) NULL) {
    //TODO: put NetException
    throw Exception(h_errno);
  }
  memcpy((void*)&(saddr->sin_addr),(void*) hp->h_addr, hp->h_length);
  
  saddr->sin_family = AF_INET;
  saddr->sin_port = htons(port);
}

void NetAddress::setAddress(std::string address) {
  this->address = address;
}

void NetAddress::setPort (int port) {
  this->port = port;
}

std::string NetAddress::getLocalHostname() {
  char hostname[256];
  gethostname(hostname,sizeof(hostname));
  return std::string(hostname);
}

std::string NetAddress::getLocalIp() {
  return NetAddress::findIp(getLocalHostname());
}


/*std::map<std::string,NetAddress> *NetAddress::getInterfaces() {
  UdpSocket socket;
  
  return socket.getLocalAddresses();
}

NetAddress NetAddress::getInterface(const char *interface) {
  UdpSocket socket;
  
  return socket.getLocalAddress(interface);
}*/

std::string NetAddress::findIp(std::string address) {
  char buff[1024];
  struct hostent *he;
  struct sockaddr_in addr;
  
  he = gethostbyname(address.c_str());
  if (he != (struct hostent*) NULL) {
    memcpy((void*)&addr.sin_addr,(void*) he->h_addr, he->h_length);
    inet_ntop(AF_INET, (void*)&(addr.sin_addr), buff, sizeof(buff));
    return std::string(buff);
  }
  return "";
}

std::string NetAddress::findHostname(const sockaddr_in &address) {
  socklen_t size;
  struct hostent *he;
  
  size = sizeof(address.sin_addr.s_addr);
  he = gethostbyaddr( (void*) &(address.sin_addr), size, AF_INET);
  if (he != (struct hostent*) NULL) {
    return he->h_name;
  }
  return "";
}

std::string NetAddress::findHostname(std::string address) {
  NetAddress addr(address,0);
  struct sockaddr_in sockAddr;

  addr.getSockAddr(&sockAddr);
  return findHostname(sockAddr);
}
