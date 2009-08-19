#include "simple_serializable.h"

#include <stdlib.h>

SimpleSerializable::SimpleSerializable() {}

SimpleSerializable::~SimpleSerializable() {}

NetMessage *SimpleSerializable::serialize() const {
  int size = returnDataSize();
  char *buff = (char*) malloc(size);
  memcpy((void*) buff, (void*)&(((char*) this)[sizeof(int)]), size);

  NetMessage *message = new NetMessage(getType(),buff,size);
  return message;
}

Serializable *SimpleSerializable::deserialize(SimpleSerializable *ss,          
  const NetMessage &data, bool ptr) {
  memcpy((void*)&(((char*) ss)[sizeof(int)]), (void*) data.getData(),
    data.getDataSize());
  
  if (ptr) {
    void **returnPtr = new void*();
    *returnPtr = (void*) ss;
    return (Serializable*) returnPtr;
  } else {
    return (Serializable*) ss;
  }
}
