#ifndef MUTEX_H
#define MUTEX_H

#include <pthread.h>

#include "exception.h"

class Condition;

class Mutex {
  typedef int Type;

  private :
    pthread_mutex_t m; 
    pthread_mutexattr_t attr;

    void init(Mutex::Type t);

    void launchLockException(int res);
    void launchTrylockException(int res);
    void launchUnlockException(int res);

    friend class Condition;

  public :
    static const Mutex::Type normalType;
    static const Mutex::Type errorcheckType;
    static const Mutex::Type recursiveType;
    static const Mutex::Type defaultType;
    
    Mutex();
    Mutex(Mutex::Type t);
    ~Mutex();
    
    inline void lock(void) {
      int res = pthread_mutex_lock(&m); 
  
      if (res != 0) {
        launchLockException(res);
      }
    }

    inline void trylock(void) {
      int res = pthread_mutex_trylock(&m); 

      if (res != 0) {
        launchTrylockException(res);
      }
    }

    inline void unlock(void) {
      int res = pthread_mutex_unlock(&m); 
       
      if (res != 0) {
        launchUnlockException(res);
      }
    }

    Condition *getNewCondition();

   class MutexException : public Exception {

      public :
        MutexException(int code, std::string message);
    };
};

#endif
