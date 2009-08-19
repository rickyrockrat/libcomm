#include "string_serializable.h"
#include "../serialization_manager.h"

#include <iostream>
#include <string.h>
#include <stdlib.h>

uint16_t String::type = 0;

int String::returnDataSize() const {
  return this->length();
}

NetMessage *String::serialize() const {
  char *data = (char*) malloc(this->length()*sizeof(char));
  memcpy(data,this->c_str(),this->length());
  return new NetMessage(getType(),data,this->length());
}

Serializable *String::deserialize(const NetMessage &data, bool ptr) {
  String *str = new String(data.getData(),data.getDataSize());
  if (ptr) {
    String **strPtr = new String*();
    *strPtr = str;
    return (Serializable*) strPtr;
  } else {
    return str;
  }
}

uint16_t String::getType() const {
  return type; 
}

String::String() : std::string() {}
String::String(const std::string &s) : std::string(s) {}
String::String(size_t length, const char &ch) : std::string(length,ch) {}
String::String(const char *str) : std::string(str) {}
String::String(const char *str, size_t length) : std::string(str,length) {}
String::String(const std::string &str, size_t index, size_t length)
  : std::string(str,index,length) {}
String::String(std::string::iterator start, std::string::iterator end)
  : std::string(start,end) {}
