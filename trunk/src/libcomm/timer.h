#ifndef TIMER_H
#define TIMER_H

#include <time.h>
#include <sys/signal.h>
#include <stdint.h>

#include "exception.h"

class Timer;

class TimerTask {
  private :
    Timer *timer;
    static void entryPoint(sigval_t val);

    friend class Timer;
  public :
    TimerTask();
    virtual void run() = 0;
    virtual ~TimerTask();
};

class Timer {
  private :
    timer_t t;
    TimerTask *task;
    struct itimerspec val; 
    struct itimerspec checkPoint; 
    struct sigevent sigEvent;
    //union sigval sigVal;
    //struct sigaction sigAct;
    bool created;
    bool running;
    bool periodic;

    void createTimer();

    friend class TimerTask;
  public :
    Timer();
    Timer(TimerTask *task);
    ~Timer();
    
    void setTask(TimerTask *task);
    void startTimer(time_t sec, long nanosec);
    void startTimerWithPeriod(time_t secPeriod, long nanosecPeriod,
                    time_t secExpiration, long nanosecExpiration);
    void startTimer(uint64_t nanosec);
    void startTimerWithPeriod(uint64_t nanosecPeriod,
                    uint64_t nanosecExpiration);
    void startTimer();
    void doCheckPoint();
    void getTime(time_t &sec, long &nanosec); 
    uint64_t getTime();
    void getDiffTime(time_t &sec, long &nanosec, bool fromStart);
    uint64_t getDiffTime(bool fromStart);
    void stopTimer();
    bool isRunning(void);

    class TimerException : public Exception {

      public :
        TimerException(int code);
        TimerException(int code, std::string message);
    };

};

#endif
