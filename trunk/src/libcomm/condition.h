#ifndef CONDITION_H
#define CONDITION_H

#include <pthread.h>
#include <stdint.h>

#include "exception.h"
#include "mutex.h"
#include <sys/time.h>
#define NB_NSEC_IN_SEC 1000000000

class Mutex;

class Condition {
  private :
    pthread_cond_t c;
    Mutex *m;
    Condition(Mutex *m);

    friend class Mutex;
  public :
    ~Condition();

    inline void wait(void) {
      int res = pthread_cond_wait(&c,&(m->m));

      if (res != 0) {
        throw Condition::ConditionException(res);
      }
    }

    inline void timedWait(time_t sec, long nanosec) {
      struct timeval tv;
      struct timespec t;

      gettimeofday(&tv,NULL);

      t.tv_sec = tv.tv_sec + sec;
      t.tv_nsec = tv.tv_usec * 1000 + nanosec;
      
      int res = pthread_cond_timedwait(&c,&(m->m),&t);

      if (res != 0) {
        throw Condition::ConditionException(res);
      }
    }

    inline void timedWait(uint64_t nanosec) {
      time_t sec = nanosec / NB_NSEC_IN_SEC;
      long nsec = nanosec % NB_NSEC_IN_SEC;
      timedWait(sec,nsec);
    }

    inline void notify(void) {
      int res = pthread_cond_signal(&c);

      if (res != 0) {
        throw Condition::ConditionException(res);
      }
    }

    inline void notifyAll(void) {
      int res = pthread_cond_broadcast(&c);

      if (res != 0) {
        throw Condition::ConditionException(res);
      }
    }

   class ConditionException : public Exception {

      public :
        ConditionException(int code);
        ConditionException(int code, std::string message);
    };
};

#endif
