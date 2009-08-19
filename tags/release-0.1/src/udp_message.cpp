#include <string>
#include <time.h>
#include <netinet/in.h>  // htonl
#include <string.h> //memcpy
#include <stdlib.h> //malloc

#include "udp_message.h"
#include "types_utils.h"
#include "exception.h"

#include <iostream>

UdpMessage::UdpMessage(uint16_t type, char *data, uint16_t dataSize) {
  timeStamp = time(NULL);

  this->type = type;
  this->data = data;  
  this->dataSize = dataSize;
}

#include <iostream>
UdpMessage::UdpMessage(const char* buffer, size_t bufferAvailable,
  size_t *byteRead, bool fetchTs) {
  size_t size_uint16_t = sizeof(uint16_t);
  size_t size_uint64_t = sizeof(uint64_t);
  size_t size_headers = 2*size_uint16_t;

  int tsSize = 0;
  if (fetchTs) {
    tsSize = size_uint64_t;
    size_headers += tsSize;
  }
  
  if (bufferAvailable < size_headers) {
    throw (Exception(0,"Not enough data for headers"));
  }

  if (fetchTs) {
    timeStamp = (time_t) convertToUInt64(buffer);
  }

  type = convertToUInt16(&(buffer[tsSize]));
  dataSize = convertToUInt16(&(buffer[tsSize+size_uint16_t]));
  
  size_t dataRemaining = bufferAvailable-size_headers;
  if (dataSize > (dataRemaining)) {
    throw (Exception(0,"Not enough data for content"));
  }

  data = (char*) malloc(dataSize*sizeof(char));
  memcpy(data,&(buffer[size_headers]),dataSize);
  
  if (byteRead != NULL) {
    *byteRead = size_headers + dataSize;
  }
}

UdpMessage::~UdpMessage() {
  free(data);
}

void UdpMessage::getRawRepresentation(bool withTs, char *c) const {
  size_t size_uint16_t = sizeof(uint16_t);
  size_t size_uint64_t = sizeof(uint64_t);
  if (withTs) {
    ((sizeof(time_t) == sizeof(uint32_t))
        ? convertToChars(((uint64_t) timeStamp & 0xFFFFFFFF),c)
        : convertToChars((uint64_t) timeStamp,c));
  }
  convertToChars(type,&(c[withTs ? size_uint64_t : 0]));
  convertToChars(dataSize,&(c[(withTs ? size_uint64_t:0)+size_uint16_t]));
  memcpy(&(c[(withTs?size_uint64_t:0)+2*size_uint16_t]),data,dataSize);
}

char *UdpMessage::getRawRepresentation(bool withTs, size_t &size) const {
  size_t totalSize = 2*sizeof(uint16_t)+
    ((withTs) ? sizeof(uint64_t) : 0 ) + dataSize;
  char *returnChars = (char*) malloc(totalSize);
  memset((void*) returnChars, 0, totalSize);
  getRawRepresentation(withTs,returnChars);
  size = totalSize;
  return returnChars;
}

uint16_t UdpMessage::getType() const {
  return type;
}

void UdpMessage::setType(uint16_t type) {
  this->type = type;
}

uint16_t UdpMessage::getDataSize() const {
  return dataSize;
}

time_t UdpMessage::getTimeStamp() const {
  return timeStamp;
}

const char *UdpMessage::getData() const {
  return data;  
}

void UdpMessage::addData(uint16_t type, const char *newData, size_t size) {
  size_t size_uint16_t = sizeof(uint16_t);
  size_t totalSize = dataSize + size + 2*size_uint16_t;
  data = (char*) realloc((void*) data,totalSize);
  convertToChars(type,&(data[dataSize]));
  convertToChars((uint16_t)size,&(data[dataSize+size_uint16_t]));
  memcpy(&(data[dataSize+2*size_uint16_t]),newData,size);
  dataSize = totalSize;
}

void UdpMessage::addData(const UdpMessage &message) {
  size_t totalSize = dataSize + message.getDataSize() + 2*sizeof(uint16_t);
  data = (char*) realloc((void*) data,totalSize);
  message.getRawRepresentation(false,&(data[dataSize]));
  dataSize = totalSize;
}

void UdpMessage::addData(const char *newData, size_t size) {
  size_t totalSize = dataSize + size;
  data = (char*) realloc((void*) data,totalSize);
  memcpy(&(data[dataSize]),newData,size);
  dataSize = totalSize;
}
