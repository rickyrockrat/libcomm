#include "mutex.h"
#include <errno.h>
#include "condition.h"

const Mutex::Type Mutex::normalType = PTHREAD_MUTEX_NORMAL;
const Mutex::Type Mutex::errorcheckType = PTHREAD_MUTEX_ERRORCHECK;
const Mutex::Type Mutex::recursiveType = PTHREAD_MUTEX_RECURSIVE;
const Mutex::Type Mutex::defaultType = PTHREAD_MUTEX_DEFAULT;

Mutex::Mutex() {
  init(-1);
}

Mutex::Mutex(Mutex::Type t) {
  init(t);
}

void Mutex::init(Mutex::Type t) {
  int res = 0;
  res = pthread_mutexattr_init(&attr);

  if (res != 0) {
    std::string errorMessage = "";
    switch (res) {
      case ENOMEM :
        errorMessage += "Insufficient memory exists to initialize the attibute";
        errorMessage += "object.";
        break;
    }
    throw MutexException(res,errorMessage);
  }

  switch (t) {
    case -1 :
    case Mutex::normalType | Mutex::defaultType :
    case Mutex::errorcheckType :
    case Mutex::recursiveType :
      pthread_mutexattr_settype(&attr, t);
      res = pthread_mutex_init(&m, &attr);
      break;
    default :
      res = pthread_mutex_init(&m, NULL);
      break;
  }

  if (res != 0) {
    throw Mutex::MutexException(res);
  }
}

Mutex::~Mutex() {
  int res = pthread_mutex_destroy(&m); 
   
  if (res != 0) {
    throw Mutex::MutexException(res);
  }

  res = pthread_mutexattr_destroy(&attr);
  
  if (res != 0) {
    throw Mutex::MutexException(res);
  }

}

Condition *Mutex::getNewCondition() {
  return new Condition(this);
}

Mutex::MutexException::MutexException(int code, std::string message)
  : Exception(code,message) {}

Mutex::MutexException::MutexException(int code): Exception(code) {}
