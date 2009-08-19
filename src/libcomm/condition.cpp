#include "condition.h"
#include <errno.h>


Condition::Condition(Mutex *m) {
  this->m = m;
  int res = pthread_cond_init(&c,NULL);

  if (res != 0) {
    throw Condition::ConditionException(res);
   }
}

Condition::~Condition() {
  int res = pthread_cond_destroy(&c); 
   
  if (res != 0) {
    throw Condition::ConditionException(res);
  }

}
Condition::ConditionException::ConditionException(int code, std::string message)
: Exception(code,message) {}

Condition::ConditionException::ConditionException(int code) : Exception(code) {}
