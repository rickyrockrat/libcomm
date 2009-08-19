#include "auto_serializable.h"
#include "stdarg.h"
#include "../types_utils.h"

AutoSerializable::AutoSerializable() {}

AutoSerializable::~AutoSerializable() {} 

char *AutoSerializable::cvrtochars(uint8_t uint, size_t &size) {
  return convertToChars(uint,size);
}

char *AutoSerializable::cvrtochars(uint16_t uint, size_t &size) {
  return convertToChars(uint,size);
}

char *AutoSerializable::cvrtochars(uint32_t uint, size_t &size) {
  return convertToChars(uint,size);
}

char *AutoSerializable::cvrtochars(uint64_t uint, size_t &size) {
  return convertToChars(uint,size);
}

char *AutoSerializable::cvrtochars(int8_t i, size_t &size) {
  return convertToChars((uint8_t) i, size);
}

char *AutoSerializable::cvrtochars(int16_t i, size_t &size) {
  return convertToChars((uint16_t) i, size);
}

char *AutoSerializable::cvrtochars(int32_t i, size_t &size) {
  return convertToChars((uint32_t) i, size);
}

char *AutoSerializable::cvrtochars(int64_t i, size_t &size) {
  return convertToChars((uint64_t) i, size);
}

char *AutoSerializable::cvrtochars(float f, size_t &size) {
  return convertToChars(f, size);
}

char *AutoSerializable::cvrtochars(double d, size_t &size) {
  return convertToChars(d, size);
}
