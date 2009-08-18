#ifndef TYPES_UTILS_H
#define TYPES_UTILS_H

#include <stdint.h>

enum {  NONE,
        CHAR,
        FLOAT,
        DOUBLE,
        INT8_T,
        INT16_T,
        INT32_T,
        INT64_T,
        UINT8_T,
        UINT16_T,
        UINT32_T,
        UINT64_T,
        NB_PRIMITVE_TYPES
};

union swap_float {
  float f;
  char chars[4];
};

union swap_double {
  double d;
  char chars[8];
};

union swap_16 {
  uint16_t uint;
  char chars[2];
};

union swap_32 {
  uint32_t uint;
  char chars[4];
};

union swap_64 {
  uint64_t uint;
  char chars[8];
};

int isLittleEndian();

void convertToChars(uint8_t uint, char *c);
char *convertToChars(uint8_t uint, size_t &size);
void convertToChars(uint16_t uint, char *c);
char *convertToChars(uint16_t uint, size_t &size);
void convertToChars(uint32_t uint, char *c);
char *convertToChars(uint32_t uint, size_t &size);
void convertToChars(uint64_t uint, char *c);
char *convertToChars(uint64_t uint, size_t &size);
void convertToChars(float f, char *c);
char *convertToChars(float f, size_t &size);
void convertToChars(double d, char *c);
char *convertToChars(double d, size_t &size);
uint8_t convertToUInt8(const char* buf);
uint16_t convertToUInt16(const char* buf);
uint32_t convertToUInt32(const char* buf);
uint64_t convertToUInt64(const char* buf);
float convertToFloat(const char* buf);
double convertToDouble(const char* buf);

#endif
