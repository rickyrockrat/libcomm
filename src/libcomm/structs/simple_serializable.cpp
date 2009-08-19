#include "simple_serializable.h"

#include <stdlib.h>

SimpleSerializable::SimpleSerializable() {}

SimpleSerializable::~SimpleSerializable() {}

NetMessage *SimpleSerializable::serialize() const {
  int size = returnClassSize();
  char *buff = (char*) malloc(size);
  memcpy((void*) buff, (void*)&(((char*) this)[sizeof(int)]), size);

  NetMessage *message = new NetMessage(getType());
  message->addDataBlock(buff, size, true);
  return message;
}

Serializable *SimpleSerializable::deserialize(SimpleSerializable *ss,          
  const NetMessage &data, bool ptr) {
  chariovec *iov = data.getDataBlocks();
  memcpy((void*)&(((char*) ss)[sizeof(int)]), (void*) iov[0].iov_base, iov[0].iov_len);
  free(iov);
  
  if (ptr) {
    void **returnPtr = new void*();
    *returnPtr = (void*) ss;
    return (Serializable*) returnPtr;
  } else {
    return (Serializable*) ss;
  }
}
