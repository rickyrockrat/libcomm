#include "auto_serializable.h"
#include "stdarg.h"
#include "../types_utils.h"

AutoSerializable::AutoSerializable() {}

AutoSerializable::~AutoSerializable() {} 

char *AutoSerializable::cvrtochars(uint8_t uint, size_t &size) {
  return convertToChars(uint,size);
}

void AutoSerializable::cvrtochars(uint8_t uint, char *data, size_t &size) {
  convertToChars(uint, data);
  size += sizeof(uint);
}

char *AutoSerializable::cvrtochars(uint16_t uint, size_t &size) {
  return convertToChars(uint,size);
}

void AutoSerializable::cvrtochars(uint16_t uint, char *data, size_t &size) {
  convertToChars(uint, data);
  size += sizeof(uint);
}

char *AutoSerializable::cvrtochars(uint32_t uint, size_t &size) {
  return convertToChars(uint,size);
}

void AutoSerializable::cvrtochars(uint32_t uint, char *data, size_t &size) {
  convertToChars(uint, data);
  size += sizeof(uint);
}

char *AutoSerializable::cvrtochars(uint64_t uint, size_t &size) {
  return convertToChars(uint,size);
}

void AutoSerializable::cvrtochars(uint64_t uint, char *data, size_t &size) {
  convertToChars(uint, data);
  size += sizeof(uint);
}

char *AutoSerializable::cvrtochars(int8_t i, size_t &size) {
  return convertToChars((uint8_t) i, size);
}

void AutoSerializable::cvrtochars(int8_t i, char *data, size_t &size) {
  convertToChars((uint8_t) i, data);
  size += sizeof(i);
}

char *AutoSerializable::cvrtochars(int16_t i, size_t &size) {
  return convertToChars((uint16_t) i, size);
}

void AutoSerializable::cvrtochars(int16_t i, char *data, size_t &size) {
  convertToChars((uint16_t) i, data);
  size += sizeof(i);
}

char *AutoSerializable::cvrtochars(int32_t i, size_t &size) {
  return convertToChars((uint32_t) i, size);
}

void AutoSerializable::cvrtochars(int32_t i, char *data, size_t &size) {
  convertToChars((uint32_t) i, data);
  size += sizeof(i);
}

char *AutoSerializable::cvrtochars(int64_t i, size_t &size) {
  return convertToChars((uint64_t) i, size);
}

void AutoSerializable::cvrtochars(int64_t i, char *data, size_t &size) {
  convertToChars((uint64_t) i, data);
  size += sizeof(i);
}

char *AutoSerializable::cvrtochars(float f, size_t &size) {
  return convertToChars(f, size);
}

void AutoSerializable::cvrtochars(float f, char *data, size_t &size) {
  convertToChars(f, data);
  size += sizeof(f);
}

char *AutoSerializable::cvrtochars(double d, size_t &size) {
  return convertToChars(d, size);
}

void AutoSerializable::cvrtochars(double d, char *data, size_t &size) {
  convertToChars(d, data);
  size += sizeof(d);
}
