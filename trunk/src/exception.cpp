#include "exception.h"

#include <iostream>
#include <string.h>

Exception::Exception(void): code(0), message("") {}

Exception::Exception(int code): code(code) {
  char buffer[1024];
  char *str;

  str = strerror_r(code, buffer, 1024);
  message = std::string(str);
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
