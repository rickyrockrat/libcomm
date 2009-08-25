//! \file thread.h
//! \brief Thread class
//! \author Alexandre Roux
//! \version 0.1
//! \date 13 mars 2009
//!
//! File containing the declarations of the class Thread.
#ifndef THREAD_H
#define THREAD_H

//! \class Thread libcomm/thread.h
//! \brief Thread class.
//!
//! Wrapper on the pthreads library for threads. Note that
//! the class is not complete, some function will be added in the future like
//! kill, exit, equal, cancel ...
//!
//! Important note: to release internal pthread memory, the join method must be
//! called before deleting the Thread. Since join is a blocking call (waiting for
//! the thread to finish) it may be sometimes wired to call it properly.
//! You can use clean to elegantly join and delete a Thread from itself.
#include "pthread.h"
#include "exception.h"

class ThreadGarbageCollector;

class Thread {

  public:

    //! \brief Thread constuctor
    //!
    //! Creates a new Thread.
    Thread(void);

    //! \brief Thread destructor
    //!
    //! Destroies the current thread.
    virtual ~Thread(void);

    //! \brief Starts the thread
    //!
    //! Starts the current thread. Calls the overloaded Thread::run(void) method.
    void start(void);
    
    //! \brief Joins the thread
    //! \return the thread return
    //!
    //! Waits for the thread to finish.
    void *join(void);

    //! \brief Cancels the current running thread
    //!
    //! The current running thread is canceled and exits. Note that the thread
    //! may be canceled in an asynchronous way. Call Thread::join to be sure
    //! that the thread has stopped.
    void cancel(void);

    class ThreadException : public Exception {

      public :
        ThreadException(int code);
        ThreadException(int code, std::string message);
    };
  
    //! \brief Cleans the memory used by the ThreadGarbageCollector
    //!
    //! Cleans the memory used by the ThreadGarbageCollector. It is called by the
    //! libcomm::clean method. Thus, this method should not be called directly.
    static void cleanup(void);
  protected:
    
    //! \brief Thread running function
    //! \return The return value of the join function
    //!
    //! This method shall contains the running code of the thread. It
    //! shall return a value (which can be NULL) which is returned by
    //! the Thread::join method.
    virtual void *run(void) = 0;
    
    //! \brief Joins and frees the current thread. 
    //!
    //! This method sets the current thread to garbage. It means that it will be
    //! eventually joined and deleted by the ThreadGarbageCollector. This method
    //! has been intended to be called only at the end of the run method and for
    //! dynamically allocated Thread (with new). 
    void clean(void);
   
  private:
    static ThreadGarbageCollector *threadGarbageCollector;

    static void *entryPoint(void *pthis);
    pthread_t threadId;

    friend class libcomm;

};

#endif
