#include "timer.h"

#include <string>
#include <string.h>
#include <errno.h>

#define NB_NSEC_IN_SEC 1000000000

void TimerTask::entryPoint(sigval_t val) {
  TimerTask *tsk = (TimerTask*) val.sival_ptr;
  tsk->timer->running = false;
  tsk->run();
}

TimerTask::TimerTask() {}

TimerTask::~TimerTask() {}

Timer::Timer() : created(false), running(false) {
  task = (TimerTask*) NULL;
}

Timer::Timer(TimerTask *task) : created(false), running(false) {
  this->task = task;
  task->timer = this;
}

Timer::~Timer() {
  if (created) {
    running = false;
    timer_delete(t);
    created = false;
  }
  delete task;
}

void Timer::setTask(TimerTask *task) {
  stopTimer();
  this->task = task;
  task->timer = this;
  createTimer();
}

void Timer::createTimer() {
  memset(&(sigEvent),0,sizeof(sigevent));
  sigEvent.sigev_notify = (task == NULL) ? SIGEV_NONE : SIGEV_THREAD;
  if (task != NULL) {
    sigEvent.sigev_notify_function = TimerTask::entryPoint;
    sigEvent.sigev_value.sival_ptr = (void*) task;
  }

  long res = timer_create(CLOCK_REALTIME,&sigEvent,&t);

  if (res != 0) {
    std::string errorMessage = "";
    switch (res) {
      case EINVAL :
        errorMessage += "An invalid which_clock value was specified.";
        errorMessage += "(Should never happen!)";
        throw Timer::TimerException(res,errorMessage); 
      case EAGAIN :
        errorMessage += "The system could not process the request.";
        throw Timer::TimerException(res,errorMessage); 
      case EFAULT :
        errorMessage += "An invalid timer_event_spec value was specified.";
        errorMessage += "(Should never happen!)";
        throw Timer::TimerException(res,errorMessage); 
    }
  }

  created = true;
}

void Timer::startTimer(time_t sec, long nanosec) {
  startTimerWithPeriod(0,0,sec,nanosec);
}

void Timer::startTimerWithPeriod(time_t secPeriod, long nanosecPeriod,
                time_t secExpiration, long nanosecExpiration) {
  if (running) stopTimer();
  if (!created) createTimer();
  memset(&val, 0, sizeof(val)); 
  val.it_interval.tv_sec = secPeriod;
  val.it_interval.tv_nsec = nanosecPeriod;
  val.it_value.tv_sec = secExpiration;
  val.it_value.tv_nsec = nanosecExpiration; 
  memcpy(&checkPoint, &val, sizeof(val));

  long res = timer_settime(t,0, &val, NULL);
  if (res != 0) {
    std::string errorMessage = "";
    switch (res) {
      case EINVAL :
        errorMessage += "An invalid timer_id value was specified.";
        errorMessage += "(Should never happen!)";
        throw Timer::TimerException(res,errorMessage); 
      case EFAULT :
        errorMessage += "The new_setting valid is invalid.";
        errorMessage += "(Should never happen!)";
        throw Timer::TimerException(res,errorMessage); 
    }
  }

  running = true;
}

void Timer::startTimer(uint64_t nanosec) {
  time_t sec = nanosec / NB_NSEC_IN_SEC;
  long nsec = nanosec % NB_NSEC_IN_SEC;
  startTimer(sec,nsec);
}

void Timer::startTimerWithPeriod( uint64_t nanosecPeriod, 
  uint64_t nanosecExpiration) {

  time_t secP = nanosecPeriod / NB_NSEC_IN_SEC;
  long nsecP = nanosecPeriod % NB_NSEC_IN_SEC;
  time_t secE = nanosecExpiration / NB_NSEC_IN_SEC;
  long nsecE = nanosecExpiration % NB_NSEC_IN_SEC;
  startTimerWithPeriod(secP,nsecP,secE,nsecE);
}


void Timer::startTimer() {
  startTimer(1000000000,0);
}

void Timer::doCheckPoint() {
  long res = timer_gettime(t,&checkPoint);
  if (res != 0) {
    launchGetTimeException(res);
  }
}

void Timer::getTime(time_t &sec, long &nanosec) {
  struct itimerspec valTemp;
  long res = timer_gettime(t,&valTemp);
  if (res != 0) {
    launchGetTimeException(res);
  } else {
    sec = valTemp.it_value.tv_sec;
    nanosec = valTemp.it_value.tv_nsec;
  }
}

uint64_t Timer::getTime() {
  time_t sec = 0;
  long nsec = 0;
  getTime(sec,nsec);
  uint64_t returnNsec = nsec;
  uint64_t returnSec = sec;
  returnSec *= NB_NSEC_IN_SEC;
  returnNsec += returnSec;
  return returnNsec;
}

void Timer::getDiffTime(time_t &sec, long &nanosec, bool fromStart) {
  struct itimerspec *v = (fromStart) ? &val : &checkPoint;
  struct itimerspec valTemp;
  long res = timer_gettime(t,&valTemp);
  if (res != 0) {
    launchGetTimeException(res);
  } else {
    if (valTemp.it_value.tv_nsec > v->it_value.tv_nsec) {
      sec = v->it_value.tv_sec - valTemp.it_value.tv_sec - 1;
      nanosec = v->it_value.tv_nsec + (NB_NSEC_IN_SEC - valTemp.it_value.tv_nsec);
    } else {
      sec = v->it_value.tv_sec - valTemp.it_value.tv_sec;
      nanosec = v->it_value.tv_nsec - valTemp.it_value.tv_nsec;
    }
  }
}

uint64_t Timer::getDiffTime(bool fromStart) {
  time_t sec = 0;
  long nsec = 0;
  getDiffTime(sec,nsec, fromStart);
  uint64_t returnNsec = nsec;
  uint64_t returnSec = sec;
  returnSec *= NB_NSEC_IN_SEC;
  returnNsec += returnSec;
  return returnNsec;
}

void Timer::stopTimer() {
  if (created) {
    running = false;
    timer_delete(t);
    created = false;
  }
}

bool Timer::isRunning(void) {
  return running;
}

void Timer::launchGetTimeException(long res) {
  std::string errorMessage = "";
  switch (res) {
    case EINVAL :
      errorMessage += "An invalid timer_id value was specified.";
      errorMessage += "(Should never happen!)";
      break;
    case EFAULT :
      errorMessage += "An invalid address of setting was specified.";
      errorMessage += "(Should never happen!)";
      break;
    default :
      errorMessage += "Unknown error.";
      break;
  }
  throw Timer::TimerException(res,errorMessage); 
}

Timer::TimerException::TimerException(int code, std::string message)
  : Exception(code,message) {}