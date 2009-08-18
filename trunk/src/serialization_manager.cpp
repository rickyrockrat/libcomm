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
  NetMessage *cloneMessage;
  void *copy;

  message = object.serialize();
  //std::cout << "message:" << message << std::endl;
  cloneMessage = new NetMessage(*message);
  //std::cout << "clone message:" << cloneMessage << std::endl;
  copy = deserialize(*cloneMessage, false);
  //std::cout << "copy:" << copy << std::endl;
  delete message;
  delete cloneMessage;

  return (Serializable*) copy;
}

Serializable *SerializationManager::clone(const Serializable *object) {
  return clone(*object);
}

void *SerializationManager::deserialize(const NetMessage *message, bool ptr) {
  return deserialize(*message, ptr);
}

void *SerializationManager::deserialize(const NetMessage &message, bool ptr) {
  uint16_t min = 0;
  uint16_t max = NB_PRIMITVE_TYPES;
  uint16_t type = message.getType();
  
  std::cout << "Search deserialization func: " << type << std::endl;
  if ((type >= min) && (type < max)) {
    //std::cout << "Primitive type: " << type << std::endl;
    return deserializePrimitive(message);
  } else {
    std::map<uint16_t, DeserializeFunc>::iterator iter = 
      deserializeFuncs.find(type);
    if (iter != deserializeFuncs.end()) {
      std::cout << "Deserialize function found: " << type << std::endl;
      DeserializeFunc f = iter->second;
      //std::cout << "Deserialize function found: " << type << std::endl;
      return (void*) f(message, ptr);
    } else {
      std::cout << "No deserialize function found: " << type << std::endl;
      return (void*) NULL;
    }
  }
}

void *SerializationManager::deserializePrimitive(const NetMessage &message) {
  switch (message.getType()) {
    case CHAR :
      return (void*) new char(*message.getPrimitiveValue());
    case FLOAT :
      return (void*) new float(convertToFloat(message.getPrimitiveValue()));
    case DOUBLE :
      return (void*) new double(convertToDouble(message.getPrimitiveValue()));
    case INT8_T :
      return (void*) new int8_t((int8_t) convertToUInt8(message.getPrimitiveValue()));
    case INT16_T :
      return (void*) new int16_t((int16_t) convertToUInt16(message.getPrimitiveValue()));
    case INT32_T :
      return (void*) new int32_t((int32_t) convertToUInt32(message.getPrimitiveValue()));
    case INT64_T :
      return (void*) new int64_t((int64_t) convertToUInt64(message.getPrimitiveValue()));
    case UINT8_T :
      return (void*) new uint8_t(convertToUInt8(message.getPrimitiveValue()));
    case UINT16_T :
      return (void*) new uint16_t(convertToUInt16(message.getPrimitiveValue()));
    case UINT32_T :
      return (void*) new uint32_t(convertToUInt32(message.getPrimitiveValue()));
    case UINT64_T :
      return (void*) new uint64_t(convertToUInt64(message.getPrimitiveValue()));
    default :
      return (void*) NULL;
  }
}

NetMessage *SerializationManager::mergeMessage(NetMessage *message, NetMessage *newMessage) const {
  if (message != NULL) {
    message->addMessage(newMessage);
    return message;
  } else {
    return newMessage;
  }
}


NetMessage *SerializationManager::serialize(const Serializable &object,
  NetMessage *message) const {
  std::cout << "Serialize object" << std::endl;
  NetMessage *newMessage = object.serialize();
  return mergeMessage(message, newMessage);
}

NetMessage *SerializationManager::serialize(const Serializable *object,
  NetMessage *message) const {
  NetMessage *newMessage = object->serialize();
  return mergeMessage(message, newMessage);
}

NetMessage *SerializationManager::serialize(char c, NetMessage *message) const {
  size_t size = 0;
  char *cptr = convertToChars((uint8_t)c,size);
  NetMessage *newMessage = new NetMessage(CHAR,cptr,size);
  return mergeMessage(message, newMessage);
}

NetMessage *SerializationManager::serialize(float f, NetMessage *message) const {
  size_t size = 0;
  char *cptr = convertToChars(f,size);
  NetMessage *newMessage = new NetMessage(FLOAT,cptr,size);
  return mergeMessage(message, newMessage);
}

NetMessage *SerializationManager::serialize(double d, NetMessage *message) const {
  size_t size = 0;
  char *cptr = convertToChars(d,size);
  NetMessage *newMessage = new NetMessage(DOUBLE,cptr,size);
  return mergeMessage(message, newMessage);
}

NetMessage *SerializationManager::serialize(int8_t i, NetMessage *message) const {
  size_t size = 0;
  char *cptr = convertToChars((uint8_t)i,size);
  NetMessage *newMessage = new NetMessage(INT8_T, cptr, size);
  return mergeMessage(message, newMessage);
}

NetMessage *SerializationManager::serialize(int16_t i, NetMessage *message) const {
  size_t size = 0;
  char *cptr = convertToChars((uint16_t)i,size);
  NetMessage *newMessage = new NetMessage(INT16_T,cptr,size);
  return mergeMessage(message, newMessage);
}

NetMessage *SerializationManager::serialize(int32_t i, NetMessage *message) const {
  size_t size = 0;
  char *cptr = convertToChars((uint32_t)i,size);
  NetMessage *newMessage = new NetMessage(INT32_T,cptr,size);
  return mergeMessage(message, newMessage);
}

NetMessage *SerializationManager::serialize(int64_t i, NetMessage *message) const {
  size_t size = 0;
  char *cptr = convertToChars((uint64_t)i,size);
  NetMessage *newMessage = new NetMessage(INT64_T,cptr,size); 
  return mergeMessage(message, newMessage);
}

NetMessage *SerializationManager::serialize(uint8_t i, NetMessage *message) const {
  size_t size = 0;
  char *cptr = convertToChars(i,size);
  NetMessage *newMessage = new NetMessage(UINT8_T, cptr,size);
  return mergeMessage(message, newMessage);
}

NetMessage *SerializationManager::serialize(uint16_t i, NetMessage *message) const {
  size_t size = 0;
  char *cptr = convertToChars(i,size);
  NetMessage *newMessage = new NetMessage(UINT16_T, cptr,size);
  return mergeMessage(message, newMessage);
}

NetMessage *SerializationManager::serialize(uint32_t i, NetMessage *message) const {
  size_t size = 0;
  char *cptr = convertToChars(i,size);
  NetMessage *newMessage = new NetMessage(UINT32_T, cptr,size);
  return mergeMessage(message, newMessage);
}

NetMessage *SerializationManager::serialize(uint64_t i, NetMessage *message) const {
  size_t size = 0;
  char *cptr = convertToChars(i,size);
  NetMessage *newMessage = new NetMessage(UINT64_T, cptr,size);
  return mergeMessage(message, newMessage);
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
