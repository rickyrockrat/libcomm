#include "thread.h"
#include <errno.h>

// TODO fix segfault join before start

void Thread::start() {
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_PROCESS);
  int code = pthread_create(&threadId, &attr, &Thread::entryPoint, this);
  pthread_attr_destroy(&attr);
  
  if (code != 0) {
    throw Thread::ThreadException(code);
  }
}

Thread::Thread() {
}

Thread::~Thread() {
  //TODO: thread exit
}

void *Thread::entryPoint(void *pthis) {
   Thread * pt = (Thread*)pthis;
   return pt->run();
}

void *Thread::join() {
  void *exitStatut;
  int code = pthread_join(threadId, &exitStatut);
  if (code != 0) {
    throw Thread::ThreadException(code);
  }
  return exitStatut;
}

void Thread::cancel(void) {
  int code;

  code = pthread_cancel(threadId);
  if (code != 0) {
    throw Thread::ThreadException(code);
  }
}

Thread::ThreadException::ThreadException(int code, std::string message)
  : Exception(code, message) {
}

Thread::ThreadException::ThreadException(int code): Exception(code) {
}
