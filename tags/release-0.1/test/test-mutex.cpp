#include "libcomm/thread.h"
#include "libcomm/mutex.h"
#include "libcomm/condition.h"

#include <time.h>
#include <iostream>

Mutex m (Mutex::recursiveType);
Condition *cond;

class Kind : public Thread {
  public :
    Kind() {};
    void *run() {
      std::cout << "I am kind, I would like the lock!" << std::endl;
      m.lock();
      m.lock();
      std::cout << "I am kind, I have the lock!" << std::endl;
      m.unlock();
      m.unlock();
      usleep(2000000);
      cond->notify();
      return NULL;
    }
};

class Nasty : public Thread {
  public :
    Nasty() {};
    void *run() {
      std::cout << "I am Nasty, I will keep the lock for me!" << std::endl;
      m.lock();
      usleep(2000000); 
      std::cout << "I am Nasty, I release the lock!" << std::endl;
      m.unlock();
      usleep(1000000);
      try {
        cond->wait();
      } catch (Condition::ConditionException &e) {
        e.printCodeAndMessage();
      }
      /*usleep(2000000);
      pthread_cond_signal(&cond);*/
      return NULL;
    }
};

int main() {
  cond = m.getNewCondition();

  Kind k;
  Nasty n;
  
  n.start();
  usleep(100);
  k.start();

  n.join();
  k.join();
  delete cond;
  return 0;
}
