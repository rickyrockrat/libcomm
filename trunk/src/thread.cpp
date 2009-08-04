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
    std::string errorMessage;
    switch (code) {
      case EAGAIN :
        errorMessage =  "Impossible to create the thread because of ressources";
        errorMessage += " lack or system limitation.";
        break;
      case EINVAL :
        errorMessage = "Wrong value given for the attr param (Should never hap";
        errorMessage += "pen!)";
        break;
      case EPERM :
        errorMessage =  "Caller doesn't have appropriate permission to set the";
        errorMessage += " required scheduling parameters or scheduling policy ";
        errorMessage += "(Should never happen!)";
        break;
      default :
        errorMessage = "Unknown error";
        break;
      }
    throw (Thread::ThreadException(code,errorMessage));
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
    std::string errorMessage;
    switch (code) {
      case ESRCH :
        errorMessage = "The thread was not found.";
        break;
      case EINVAL :
        errorMessage ="The thread is not joinable.";
        break;
      case EDEADLK :
        errorMessage = "Deadlock detected (or try to join the calling thread.";
        break;
      default :
        errorMessage = "Unknown error";
        break;
    }
    throw (Thread::ThreadException(code,errorMessage));
  }
  return exitStatut;
}

void Thread::cancel(void) {
  int code;

  code = pthread_cancel(threadId);
  if (code != 0) {
    std::string errorMessage;
    switch (code) {
      case ESRCH:
        errorMessage =  "No thread could be found corresponding to that specifi"
                        "ed by the given thread ID.";
    }
    throw (Thread::ThreadException(code,errorMessage));
  }
}

Thread::ThreadException::ThreadException(int code, std::string message)
  : Exception(code, message) {
}
