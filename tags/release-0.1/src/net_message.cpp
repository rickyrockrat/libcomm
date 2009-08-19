#include <time.h>

#include "net_message.h"
#include "types_utils.h"
#include "exception.h"

NetMessage::NetMessage(uint16_t type, char *data, uint32_t dataSize) {
  timeStamp = time(NULL);

  this->type = type;
  this->data = data;  
  this->dataSize = dataSize;
}

NetMessage::NetMessage(const char* buffer, size_t bufferAvailable,
  size_t *byteRead, bool fetchTs) {
  size_t size_uint16_t = sizeof(uint16_t);
  size_t size_uint32_t = sizeof(uint32_t);
  size_t size_uint64_t = sizeof(uint64_t);
  size_t size_headers = size_uint16_t+size_uint32_t;

  int tsSize = 0;
  if (fetchTs) {
    tsSize = size_uint64_t;
    size_headers += tsSize;
  }
  
  if (bufferAvailable < size_headers) {
    throw (Exception(NET_MESSG_NEDFH,"Not enough data for headers"));
  }

  if (fetchTs) {
    timeStamp = (time_t) convertToUInt64(buffer);
  }

  type = convertToUInt16(&(buffer[tsSize]));
  dataSize = convertToUInt32(&(buffer[tsSize+size_uint16_t]));

  size_t dataRemaining = bufferAvailable-size_headers;
  if (dataSize > (dataRemaining)) {
    throw (Exception(NET_MESSG_NEDFC,"Not enough data for content"));
  }

  data = (char*) malloc(dataSize*sizeof(char));
  memcpy(data,&(buffer[size_headers]),dataSize);
  
  if (byteRead != NULL) {
    *byteRead = size_headers + dataSize;
  }
}

NetMessage::~NetMessage() {
  free(data);
}

void NetMessage::setRawData(bool withTs, char *c) const {
  size_t size_uint16_t = sizeof(uint16_t);
  size_t size_uint32_t = sizeof(uint32_t);
  size_t size_uint64_t = sizeof(uint64_t);
  if (withTs) {
    ((sizeof(time_t) == sizeof(uint32_t))
        ? convertToChars(((uint64_t) timeStamp & 0xFFFFFFFF),c)
        : convertToChars((uint64_t) timeStamp,c));
  }
  convertToChars(type,&(c[withTs ? size_uint64_t : 0]));
  convertToChars(dataSize,&(c[(withTs ? size_uint64_t:0)+size_uint16_t]));
  memcpy(&(c[(withTs?size_uint64_t:0)+size_uint16_t+size_uint32_t]),data,dataSize);
}

char *NetMessage::getRawData(bool withTs, size_t *size) const {
  size_t totalSize = sizeof(uint16_t)+sizeof(uint32_t)+
    ((withTs) ? sizeof(uint64_t) : 0 ) + dataSize;
  char *returnChars = (char*) malloc(totalSize);
  memset((void*) returnChars, 0, totalSize);
  setRawData(withTs,returnChars);
  *size = totalSize;
  return returnChars;
}

uint16_t NetMessage::getType() const {
  return type;
}

void NetMessage::setType(uint16_t type) {
  this->type = type;
}

uint32_t NetMessage::getDataSize() const {
  return dataSize;
}

time_t NetMessage::getTimeStamp() const {
  return timeStamp;
}

const char *NetMessage::getData() const {
  return data;  
}

/*
void NetMessage::addData(uint16_t type, const char *newData, size_t size) {
  size_t size_uint16_t = sizeof(uint16_t);
  size_t totalSize = dataSize + size + 2*size_uint16_t;
  data = (char*) realloc((void*) data,totalSize);
  convertToChars(type,&(data[dataSize]));
  convertToChars((uint16_t)size,&(data[dataSize+size_uint16_t]));
  memcpy(&(data[dataSize+2*size_uint16_t]),newData,size);
  dataSize = totalSize;
}*/

void NetMessage::addData(const NetMessage &message) {
  size_t totalSize = dataSize + message.getDataSize() + sizeof(uint16_t)
    + sizeof(uint32_t);
  data = (char*) realloc((void*) data,totalSize);
  message.setRawData(false,&(data[dataSize]));
  dataSize = totalSize;
}

void NetMessage::addData(const char *newData, size_t size) {
  size_t totalSize = dataSize + size;
  data = (char*) realloc((void*) data,totalSize);
  memcpy(&(data[dataSize]),newData,size);
  dataSize = totalSize;
}
