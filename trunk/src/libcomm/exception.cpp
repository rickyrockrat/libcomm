#include "exception.h"

#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

Exception::Exception(void): code(0), message("") {}

Exception::Exception(int code): code(code) {
#ifdef _GNU_SOURCE
  char buffer[1024];
  char *str;

  str = strerror_r(code, buffer, 1024);
  message = std::string(str);
#else
  char *buffer = NULL;
  size_t sizeBuffer = 1024;
  int res;
  bool cont = true;

  do {
    buffer = (char*) realloc(buffer, sizeBuffer);
    res = strerror_r(code, buffer, sizeBuffer);
    
    switch (res) {
      case 0:
        cont = false;
        message = std::string(buffer);
        break;
      case EINVAL:
        cont = false;
        message = std::string("Unknown error");
        break;
      case ERANGE:
        sizeBuffer += 1024;
        break;
    }
  } while (cont);
#endif
}

Exception::Exception(int code, std::string message): code(code), message(message) {
}

int Exception::getCode() {
  return code;
}

std::string Exception::getMessage() {
  return message;
}

void Exception::printCodeAndMessage() {
  std::cerr << "Error (" << code << "): " << message << std::endl;
}
