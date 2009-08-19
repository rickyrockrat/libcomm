#include "serialization_manager.h"
#include "types_utils.h"


#include <iostream>

SerializationManager* SerializationManager::self = (SerializationManager*) NULL;

SerializationManager::SerializationManager(void){
  currentId = NB_PRIMITVE_TYPES;
}

SerializationManager *SerializationManager::getSerializationManager(void) {
  if (self == (SerializationManager*) NULL) self = new SerializationManager();
  return self;
}


void SerializationManager::addDeserializationFunc(const uint16_t type, 
  DeserializeFunc f) {

  deserializeFuncs[type] = f;
}

uint16_t SerializationManager::addDeserializationFunc(DeserializeFunc f) {
  deserializeFuncs[currentId] = f;
  return currentId++;
}
  
Serializable *SerializationManager::clone(const Serializable &object) {
  NetMessage *message;
  void *copy;

  message = object.serialize();
  copy = deserialize(*message,false);
  delete message;

  return (Serializable*) copy;
}

Serializable *SerializationManager::clone(const Serializable *object) {
  return clone(*object);
}

void *SerializationManager::deserialize(const NetMessage &message, bool ptr) {
  uint16_t min = 0;
  uint16_t max = NB_PRIMITVE_TYPES;
  uint16_t type = message.getType();
  
  if ((type >= min) && (type < max)) {
    return deserializePrimitive(message);
  } else {
    std::map<uint16_t, DeserializeFunc>::iterator iter = 
      deserializeFuncs.find(message.getType());
    if (iter != deserializeFuncs.end()) {
      DeserializeFunc f = iter->second;
      return (void*) f(message, ptr);
    } else {
      return (void*) NULL;
    }
  }
}

void *SerializationManager::deserialize(const NetMessage &data, 
  int &currentPosition, bool ptr) {

  const char *buff = data.getData();
  size_t byteRead;
  
  NetMessage message(&(buff[currentPosition]),
    data.getDataSize()-currentPosition, &byteRead, false);

  currentPosition += byteRead;
  return deserialize(message,ptr);
}


void *SerializationManager::deserializePrimitive(const NetMessage &message) {
  switch (message.getType()) {
    case CHAR :
      return (void*) new char(*message.getData());
    case FLOAT :
      return (void*) new float(convertToFloat(message.getData()));
    case DOUBLE :
      return (void*) new double(convertToDouble(message.getData()));
    case INT8_T :
      return (void*) new int8_t((int8_t) convertToUInt8(message.getData()));
    case INT16_T :
      return (void*) new int16_t((int16_t) convertToUInt16(message.getData()));
    case INT32_T :
      return (void*) new int32_t((int32_t) convertToUInt32(message.getData()));
    case INT64_T :
      return (void*) new int64_t((int64_t) convertToUInt64(message.getData()));
    case UINT8_T :
      return (void*) new uint8_t(convertToUInt8(message.getData()));
    case UINT16_T :
      return (void*) new uint16_t(convertToUInt16(message.getData()));
    case UINT32_T :
      return (void*) new uint32_t(convertToUInt32(message.getData()));
    case UINT64_T :
      return (void*) new uint64_t(convertToUInt64(message.getData()));
    default :
      return (void*) NULL;
  }
}


NetMessage *SerializationManager::mergeMessage(NetMessage *message,
  NetMessage *toAdd) const {
   if (message == NULL) {
    return toAdd;
  } else {
    if (toAdd != NULL) {
      message->addData(*toAdd);
      delete toAdd;
      return message;
    } else {
      return (NetMessage*) NULL;
    }
  }
}

NetMessage *SerializationManager::checkAndSerialize(const Serializable &object,
  NetMessage *message) const {
  NetMessage *newPart = object.checkAndSerialize();
  return mergeMessage(message, newPart);
}

NetMessage *SerializationManager::checkAndSerialize(const Serializable *object,
  NetMessage *message) const {
  NetMessage *newPart = object->checkAndSerialize();
  return mergeMessage(message, newPart);
}

NetMessage *SerializationManager::serialize(const Serializable &object,
  NetMessage *message) const {
  NetMessage *newPart = object.serialize();
  return mergeMessage(message, newPart);
}

NetMessage *SerializationManager::serialize(const Serializable *object,
  NetMessage *message) const {
  NetMessage *newPart = object->serialize();
  return mergeMessage(message, newPart);
}

NetMessage *SerializationManager::serialize(char c, NetMessage *message) const {
  size_t size = 0;
  char *cptr = convertToChars((uint8_t)c,size);
  NetMessage *newPart = new NetMessage(CHAR,cptr,size);
  return mergeMessage(message, newPart);
}

NetMessage *SerializationManager::serialize(float f, NetMessage *message) const {
  size_t size = 0;
  char *cptr = convertToChars(f,size);
  NetMessage *newPart = new NetMessage(FLOAT,cptr,size);
  return mergeMessage(message, newPart);
}

NetMessage *SerializationManager::serialize(double d, NetMessage *message) const {
  size_t size = 0;
  char *cptr = convertToChars(d,size);
  NetMessage *newPart = new NetMessage(DOUBLE,cptr,size);
  return mergeMessage(message, newPart);
}

NetMessage *SerializationManager::serialize(int8_t i, NetMessage *message) const {
  size_t size = 0;
  char *cptr = convertToChars((uint8_t)i,size);
  NetMessage *newPart = new NetMessage(INT8_T, cptr, size);
  return mergeMessage(message, newPart);
}

NetMessage *SerializationManager::serialize(int16_t i, NetMessage *message) const {
  size_t size = 0;
  char *cptr = convertToChars((uint16_t)i,size);
  NetMessage *newPart = new NetMessage(INT16_T,cptr,size);
  return mergeMessage(message, newPart);
}

NetMessage *SerializationManager::serialize(int32_t i, NetMessage *message) const {
  size_t size = 0;
  char *cptr = convertToChars((uint32_t)i,size);
  NetMessage *newPart = new NetMessage(INT32_T,cptr,size);
  return mergeMessage(message, newPart);
}

NetMessage *SerializationManager::serialize(int64_t i, NetMessage *message) const {
  size_t size = 0;
  char *cptr = convertToChars((uint64_t)i,size);
  NetMessage *newPart = new NetMessage(INT64_T,cptr,size); 
  return mergeMessage(message, newPart);
}

NetMessage *SerializationManager::serialize(uint8_t i, NetMessage *message) const {
  size_t size = 0;
  char *cptr = convertToChars(i,size);
  NetMessage *newPart = new NetMessage(UINT8_T, cptr,size);
  return mergeMessage(message, newPart);
}

NetMessage *SerializationManager::serialize(uint16_t i, NetMessage *message) const {
  size_t size = 0;
  char *cptr = convertToChars(i,size);
  NetMessage *newPart = new NetMessage(UINT16_T, cptr,size);
  return mergeMessage(message, newPart);
}

NetMessage *SerializationManager::serialize(uint32_t i, NetMessage *message) const {
  size_t size = 0;
  char *cptr = convertToChars(i,size);
  NetMessage *newPart = new NetMessage(UINT32_T, cptr,size);
  return mergeMessage(message, newPart);
}

NetMessage *SerializationManager::serialize(uint64_t i, NetMessage *message) const {
  size_t size = 0;
  char *cptr = convertToChars(i,size);
  NetMessage *newPart = new NetMessage(UINT64_T, cptr,size);
  return mergeMessage(message, newPart);
}

int SerializationManager::getSerializedSize(const Serializable &object, 
  bool withHeaders) const {
  return ((withHeaders) ? 
    Serializable::sizeOfDataHeaders : 0) + object.returnDataSize();
}

int SerializationManager::getSerializedSize(const Serializable *object,
  bool withHeaders) const {
  return ((withHeaders) ?
    Serializable::sizeOfDataHeaders : 0 ) + object->returnDataSize();
}

int SerializationManager::getSerializedSize(char c, bool withHeaders) const {
  return ((withHeaders)? Serializable::sizeOfDataHeaders : 0 ) + sizeof(char);
}

int SerializationManager::getSerializedSize(float f, bool withHeaders) const {
  return ((withHeaders)? Serializable::sizeOfDataHeaders : 0 ) + sizeof(float);
}

int SerializationManager::getSerializedSize(double d, bool withHeaders) const {
  return ((withHeaders)? Serializable::sizeOfDataHeaders : 0 ) + sizeof(double);
}

int SerializationManager::getSerializedSize(int8_t i, bool withHeaders) const {
  return ((withHeaders)? Serializable::sizeOfDataHeaders : 0 ) + sizeof(int8_t);
}

int SerializationManager::getSerializedSize(int16_t i, bool withHeaders) const {
  return ((withHeaders)? Serializable::sizeOfDataHeaders : 0 ) + sizeof(int16_t);
}

int SerializationManager::getSerializedSize(int32_t i, bool withHeaders) const {
  return ((withHeaders)? Serializable::sizeOfDataHeaders : 0 ) + sizeof(int32_t);
}

int SerializationManager::getSerializedSize(int64_t i, bool withHeaders) const {
  return ((withHeaders)? Serializable::sizeOfDataHeaders : 0 ) + sizeof(int64_t);
}

int SerializationManager::getSerializedSize(uint8_t i, bool withHeaders) const {
  return ((withHeaders)? Serializable::sizeOfDataHeaders : 0 ) + sizeof(uint8_t);
}

int SerializationManager::getSerializedSize(uint16_t i, bool withHeaders) const {
  return ((withHeaders)? Serializable::sizeOfDataHeaders : 0 ) + sizeof(uint16_t);
}

int SerializationManager::getSerializedSize(uint32_t i, bool withHeaders) const {
  return ((withHeaders)? Serializable::sizeOfDataHeaders : 0 ) + sizeof(uint32_t);
}

int SerializationManager::getSerializedSize(uint64_t i, bool withHeaders) const {
  return ((withHeaders)? Serializable::sizeOfDataHeaders : 0 ) + sizeof(uint64_t);
}


SerializationManager::~SerializationManager(void) {
  deserializeFuncs.clear();
}


void SerializationManager::deleteT(Serializable &object) const {}

void SerializationManager::deleteT(Serializable *object) const {
  delete object;
}

void SerializationManager::deleteT(char c) const{}
void SerializationManager::deleteT(float f) const{}
void SerializationManager::deleteT(double d) const{}
void SerializationManager::deleteT(int8_t i) const{}
void SerializationManager::deleteT(int16_t i) const{}
void SerializationManager::deleteT(int32_t i) const{}
void SerializationManager::deleteT(int64_t i) const{}
void SerializationManager::deleteT(uint8_t i) const{}
void SerializationManager::deleteT(uint16_t i) const{}
void SerializationManager::deleteT(uint32_t i) const{}
void SerializationManager::deleteT(uint64_t i) const{}
