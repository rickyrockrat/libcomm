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
    std::string errorMessage = "";
    switch (res) {
      case EAGAIN : 
        errorMessage += "The system lacked the necessary resources (other than";
        errorMessage += " memory) to initialize another mutex.";
        break;
      case ENOMEM :
        errorMessage += "Insufficient memory exists to initialize the mutex.";
        break;
      case EPERM :
        errorMessage += "The caller does not have the privilege to perform the";
        errorMessage += " operation.";
        break;
      case EBUSY :
        errorMessage += "The implementation has detected an attempt to reiniti";
        errorMessage += "alize the object referenced by mutex, a previously in";
        errorMessage += "itialized, but not yet destroyed, mutex.";
        break;
      case EINVAL :
        errorMessage += "The value specified by attr is invalid (Should never ";
        errorMessage += "happen!).";
        break;
    }
    throw MutexException(res,errorMessage);
  }
}

void Mutex::launchLockException(int res) {
  std::string errorMessage = "";
  switch (res) {
    case EAGAIN : 
      errorMessage += "The mutex could not be acquired because the maximum n";
      errorMessage += "umber of recursive locks for mutex has been exceeded.";
      break;
    case EDEADLK :
      errorMessage += "The current thread already owns the mutex.";
      break;
    case EINVAL :
      errorMessage += "The mutex was created with the protocol attribute hav";
      errorMessage += "ing the value PTHREAD_PRIO_PROTECT and the calling th";
      errorMessage += "read's priority is higher than the mutex's current pr";
      errorMessage += "iority ceiling. (Should never happen!) OR The value s";
      errorMessage += "pecified by mutex does not refer to an initialized mu";
      errorMessage += "tex object (Should never happen!)";
      break;
  }
  throw MutexException(res,errorMessage);
}

void Mutex::launchTrylockException(int res) {
  std::string errorMessage = "";
  switch (res) {
    case EAGAIN : 
      errorMessage += "The mutex could not be acquired because the maximum n";
      errorMessage += "umber of recursive locks for mutex has been exceeded.";
      break;
    case EBUSY :
      errorMessage += "The mutex could not be acquired because it was alread";
      errorMessage += "y locked.";
      break;
    case EINVAL :
      errorMessage += "The mutex was created with the protocol attribute hav";
      errorMessage += "ing the value PTHREAD_PRIO_PROTECT and the calling th";
      errorMessage += "read's priority is higher than the mutex's current pr";
      errorMessage += "iority ceiling. (Should never happen!) OR The value s";
      errorMessage += "pecified by mutex does not refer to an initialized mu";
      errorMessage += "tex object (Should never happen!)";
      break;
  }
  throw MutexException(res,errorMessage);
}

void Mutex::launchUnlockException(int res) {
 std::string errorMessage = "";
  switch (res) {
    case EAGAIN : 
      errorMessage += "The mutex could not be acquired because the maximum n";
      errorMessage += "umber of recursive locks for mutex has been exceeded.";
      break;
    case EPERM :
      errorMessage += "The current thread does not own the mutex.";
      break;
    case EINVAL :
      errorMessage += "The value specified by mutex does not refer to an ini";
      errorMessage += "tialized mutex object (Should never happen!)";
      break;
  }
  throw MutexException(res,errorMessage);
}

Mutex::~Mutex() {
  int res = pthread_mutex_destroy(&m); 
   
  if (res != 0) {
    std::string errorMessage = "";
    switch (res) {
      case EBUSY : 
        errorMessage += "The implementation has detected an attempt to destroy";
        errorMessage += " the object referenced by mutex while it is locked or";
        errorMessage += " referenced (for example, while being used in a pthre";
        errorMessage += "ad_cond_timedwait() or pthread_cond_wait()) by anothe";
        errorMessage += "r thread.";
        break;
      case EINVAL :
        errorMessage += "The value specified by mutex is invalid (Should never";
        errorMessage += " happen!)";
        break;
    }
    throw MutexException(res,errorMessage);
  }

  res = pthread_mutexattr_destroy(&attr);
  
  if (res != 0) {
    std::string errorMessage = "";
    switch (res) {
      case EINVAL :
        errorMessage += "The value specified by attr is invalid (Should never ";
        errorMessage += "happen!)";
        break;
    }
    throw MutexException(res,errorMessage);
  }

}

Condition *Mutex::getNewCondition() {
  return new Condition(this);
}

Mutex::MutexException::MutexException(int code, std::string message)
  : Exception(code,message) {}
