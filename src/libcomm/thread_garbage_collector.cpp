#include "thread_garbage_collector.h"
#include "mutex.h"
#include "condition.h"

#include <errno.h>

ThreadGarbageCollector::ThreadGarbageCollector(void): go(true) {
  garbageThreadsMutex = new Mutex();
  newGarbageThread = garbageThreadsMutex->getNewCondition();
}

ThreadGarbageCollector::~ThreadGarbageCollector(void) {
  delete newGarbageThread;
  delete garbageThreadsMutex;
}

void ThreadGarbageCollector::addGarbageThread(Thread *thread) {
  garbageThreadsMutex->lock();
    garbageThreads.push(thread);
    newGarbageThread->notify();
  garbageThreadsMutex->unlock();
}

void *ThreadGarbageCollector::run(void) {
  Thread *thread;
  bool timeout;

  while ((go) || (!garbageThreads.empty())) {
    timeout = false;
    garbageThreadsMutex->lock();
      while (garbageThreads.empty()) {
        try {
          newGarbageThread->timedWait(1,0);
        } catch (Exception &e) {
          if (e.getCode() == EX_CONDITION_TIMEOUT) {
            timeout = true;
            break;
          } else {
            throw e;
          }
        }
      }
      
      if (!timeout) {
        thread = garbageThreads.front();
        garbageThreads.pop();

        try {
          thread->join();
        } catch (Exception &e) {
          // In case of the thread is already finished
        }
        delete thread;
      }
    garbageThreadsMutex->unlock();
  }

  return NULL;
}

void ThreadGarbageCollector::stop(void) {
  go = false;
}
