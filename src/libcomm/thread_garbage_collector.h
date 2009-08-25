#ifndef THREAD_GARBAGE_COLLECTOR_H
#define THREAD_GARBAGE_COLLECTOR_H

#include <queue>
#include "thread.h"

class Condition;
class Mutex;

class ThreadGarbageCollector: public Thread {
  private:
    std::queue<Thread*> garbageThreads;
    Mutex *garbageThreadsMutex;
    Condition *newGarbageThread;

    bool go;

    ThreadGarbageCollector(void);
    ~ThreadGarbageCollector(void);
    
    void addGarbageThread(Thread *thread);
    void *run(void);
    void stop(void);

    friend class Thread;
};


#endif
