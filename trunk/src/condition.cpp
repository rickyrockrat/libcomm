#include "condition.h"
#include <errno.h>


Condition::Condition(Mutex *m) {
  this->m = m;
  int res = pthread_cond_init(&c,NULL);

  if (res != 0) {
    std::string errorMessage = "";
    switch (res) {
      case EAGAIN : 
        errorMessage += "The system lacked the necessary resources (other than";
        errorMessage += " memory) to initialize another condition variable.";
        break;
      case ENOMEM :
        errorMessage += "Insufficient memory exists to initialize the conditio";
        errorMessage += "n variable.";
        break;
      case EBUSY :
        errorMessage += "The implementation has detected an attempt to reiniti";
        errorMessage += "alize the object referenced by cond, a previously ini";
        errorMessage += "tialized, but not yet destroyed, condition variable.";
        break;
      case EINVAL :
        errorMessage += "The value specified by attr is invalid (Should never ";
        errorMessage += "happen!).";
        break;
    }
    throw ConditionException(res,errorMessage);
  }
}

Condition::~Condition() {
  int res = pthread_cond_destroy(&c); 
   
  if (res != 0) {
    std::string errorMessage = "";
    switch (res) {
      case EBUSY : 
        errorMessage += "The implementation has detected an attempt to destroy";
        errorMessage += " the object referenced by cond while it is locked or ";
        errorMessage += "referenced (for example, while being used in a pthrea";
        errorMessage += "d_cond_timedwait() or pthread_cond_wait()) by another";
        errorMessage += " thread.";
        break;
      case EINVAL :
        errorMessage += "The value specified by cond is invalid (Should never ";
        errorMessage += "happen!)";
        break;
    }
    throw ConditionException(res,errorMessage);
  }

}


void Condition::launchWaitException(int res) {
  std::string errorMessage = "";
  switch (res) {
    case EINVAL :
      errorMessage += "The value specified by cond, mutex, or abstime is inv";
      errorMessage += "alid.(Should never happen!) or Different mutexes were";
      errorMessage += "supplied for concurrent pthread_cond_timedwait() or p";
      errorMessage += "thread_cond_wait() operations on the same condition v";
      errorMessage += "ariable.";
      break;
    case EPERM :
      errorMessage += "The mutex was not owned by the current thread at the ";
      errorMessage += "time of the call.";
      break;
  }
  throw ConditionException(res,errorMessage);
}

void Condition::launchTimedWaitException(int res) {
  std::string errorMessage = "";
  switch (res) {
    case ETIMEDOUT :
      errorMessage += "The time specified by abstime to pthread_cond_timedwa";
      errorMessage += "it() has passed.";
      break;
    case EINVAL :
      errorMessage += "The value specified by cond, mutex, or abstime is inv";
      errorMessage += "alid.(Should never happen!) or Different mutexes were";
      errorMessage += "supplied for concurrent pthread_cond_timedwait() or p";
      errorMessage += "thread_cond_wait() operations on the same condition v";
      errorMessage += "ariable.";
      break;
    case EPERM :
      errorMessage += "The mutex was not owned by the current thread at the ";
      errorMessage += "time of the call.";
      break;
  }
  throw ConditionException(res,errorMessage);
}

void Condition::launchNotifyException(int res) {
  std::string errorMessage = "";
  switch(res) {
    case EINVAL :
      errorMessage += "The value cond does not refer to an initialized condi";
      errorMessage += "tion variable. (Should never happen!)";
  }
}

Condition::ConditionException::ConditionException(int code, std::string message)
: Exception(code,message) {}
