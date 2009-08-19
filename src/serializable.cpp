#include "serializable.h"

const size_t Serializable::sizeOfDataHeaders = 6;
const size_t Serializable::sizeOfDataHeadersWithTs = 
  sizeOfDataHeaders + sizeof(uint64_t);

bool Serializable::checkSize(void) const{
  int udpMaxPacketSize = UDP_PACKET_MAX_SIZE;
  int etherIpHeaders = ETHER_IP_HEADERS;
  int dataHeaders = sizeOfDataHeadersWithTs;
  int totalHeaders = etherIpHeaders + dataHeaders;
  int dataSize = returnDataSize();
  
  return ((udpMaxPacketSize - dataSize - totalHeaders) >= 0 );        
}

NetMessage *Serializable::checkAndSerialize(void) const {
  if (checkSize()) {
    return serialize();
  } else {
    return (NetMessage*) NULL;
  }
}

                                                                              
Serializable::~Serializable() {}
