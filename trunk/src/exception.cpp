#include "exception.h"
#include <iostream>

Exception::Exception(int code, std::string message) {
  this->code = code;
  this->message = message;
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
