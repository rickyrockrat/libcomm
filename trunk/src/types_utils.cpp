#include <string>
#include <stdlib.h>
#include <string.h>

#include "types_utils.h"

int littleEndian = -1;

int isLittleEndian() {
  if (littleEndian == -1) {
    union swap_32 endi;
    endi.uint = 0x1;
    if (endi.chars[3] == 1) {
      littleEndian = 0;
    } else {
      littleEndian = 1;
    }
  }
  return littleEndian;
}

void convertToChars(uint8_t uint, char *c){
  *c = (char) uint;
}

char *convertToChars(uint8_t uint, size_t &size) {
  char *c = (char*) malloc(sizeof(uint8_t)*sizeof(char));
  convertToChars(uint,c);
  size = sizeof(uint8_t);
  return c;
}

void convertToChars(uint16_t uint, char *c){
  size_t size_uint16_t = sizeof(uint16_t);
  union swap_16 swp16;
  swp16.uint = uint;

  if (littleEndian) {
    for (size_t i = 0; i<size_uint16_t; ++i) {
      c[size_uint16_t-i-1] = swp16.chars[i];
    }
  } else {
    memcpy(c,swp16.chars,size_uint16_t);
  }
}

char *convertToChars(uint16_t uint, size_t &size) {
  char *c = (char*) malloc(sizeof(uint16_t)*sizeof(char));
  convertToChars(uint,c);
  size = sizeof(uint16_t)*sizeof(char);
  return c;
}

void convertToChars(uint32_t uint, char *c) {
  size_t size_uint32_t = sizeof(uint32_t);
  union swap_32 swp32;
  swp32.uint = uint;

  if (littleEndian) {
    for (size_t i = 0; i<size_uint32_t; ++i) {
      c[size_uint32_t-i-1] = swp32.chars[i];
    }
  } else {
    memcpy(c,swp32.chars,size_uint32_t);
  }
}

char *convertToChars(uint32_t uint, size_t &size) {
  char *c = (char*) malloc(sizeof(uint32_t)*sizeof(char));
  convertToChars(uint,c);
  size = sizeof(uint32_t)*sizeof(char);
  return c;
}

void convertToChars(uint64_t uint, char *c) { 
  size_t size_uint64_t = sizeof(uint64_t);
  union swap_64 swp64;
  swp64.uint = uint;

  std::string str = "";
  if (littleEndian) {
    for (size_t i = 0; i<size_uint64_t; ++i) {
      c[size_uint64_t-i-1] = swp64.chars[i];
    }
  } else {
    memcpy(c,swp64.chars,size_uint64_t);
  }
}

char *convertToChars(uint64_t uint, size_t &size) {
  char *c = (char*) malloc(sizeof(uint64_t)*sizeof(char));
  convertToChars(uint,c);
  size = sizeof(uint64_t)*sizeof(char);
  return c;
}

void convertToChars(float f, char *c) { 
  size_t size_float = sizeof(float);
  union swap_float swpf;
  swpf.f = f;

  std::string str = "";
  if (littleEndian) {
    for (size_t i = 0; i<size_float; ++i) {
      c[size_float-i-1] = swpf.chars[i];
    }
  } else {
    memcpy(c,swpf.chars,size_float);
  }
}

char *convertToChars(float f, size_t &size) {
  char *c = (char*) malloc(sizeof(float)*sizeof(char));
  convertToChars(f,c);
  size = sizeof(float)*sizeof(char);
  return c;
}

void convertToChars(double d, char *c) { 
  size_t size_double = sizeof(double);
  union swap_double swpd;
  swpd.d = d;

  std::string str = "";
  if (littleEndian) {
    for (size_t i = 0; i<size_double; ++i) {
      c[size_double-i-1] = swpd.chars[i];
    }
  } else {
    memcpy(c,swpd.chars,size_double);
  }
}

char *convertToChars(double d, size_t &size) {
  char *c = (char*) malloc(sizeof(double)*sizeof(char));
  convertToChars(d,c);
  size = sizeof(double)*sizeof(char);
  return c;
}

uint8_t convertToUInt8(const char* buf) {
  return (uint8_t) buf[0];
}

uint16_t convertToUInt16(const char* buf) {
  size_t size_uint16_t = sizeof(uint16_t);
  union swap_16 swp16;

  if (littleEndian) {
    for (size_t i = 0; i<size_uint16_t; ++i) {
      swp16.chars[i] = buf[size_uint16_t-i-1];
    }
  } else {
    memcpy (swp16.chars, buf, size_uint16_t);
  }
  
  return swp16.uint;
}

uint32_t convertToUInt32(const char* buf) {
  size_t size_uint32_t = sizeof(uint32_t);
  union swap_32 swp32;

  if (littleEndian) {
    for (size_t i = 0; i<size_uint32_t; ++i) {
      swp32.chars[i] = buf[size_uint32_t-i-1];
    }
  } else {
    memcpy (swp32.chars, buf, size_uint32_t);
  }
  
  return swp32.uint;
}

uint64_t convertToUInt64(const char* buf) {
  size_t size_uint64_t = sizeof(uint64_t);
  union swap_64 swp64;

  if (littleEndian) {
    for (size_t i = 0; i<size_uint64_t; ++i) {
      swp64.chars[i] = buf[size_uint64_t-i-1];
    }
  } else {
    memcpy (swp64.chars, buf, size_uint64_t);
  }
  
  return swp64.uint;
}

float convertToFloat(const char* buf) {
  size_t size_float = sizeof(float);
  union swap_float swpf;

  if (littleEndian) {
    for (size_t i = 0; i<size_float; ++i) {
      swpf.chars[i] = buf[size_float-i-1];
    }
  } else {
    memcpy (swpf.chars, buf, size_float);
  }
  
  return swpf.f;
}

double convertToDouble(const char* buf) {
  size_t size_double = sizeof(double);
  union swap_double swpd;

  if (littleEndian) {
    for (size_t i = 0; i<size_double; ++i) {
      swpd.chars[i] = buf[size_double-i-1];
    }
  } else {
    memcpy (swpd.chars, buf, size_double);
  }
  
  return swpd.d;
}
