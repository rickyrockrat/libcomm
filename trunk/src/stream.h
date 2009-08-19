#ifndef STREAM_H
#define STREAM_H

#include <sys/time.h> 
#include <vector>
#include <stdint.h>

#include "exception.h"

enum StreamWFRSet {
  STREAM_WFR_NONE = 0,
  STREAM_WFR_READ = 1,
  STREAM_WFR_WRITE = 2,
  STREAM_WFR_READ_WRITE = 3,
  STREAM_WFR_ERROR = 4,
  STREAM_WFR_ALL = 7
};

class StreamWFRResult;

class Stream {
  protected:
    int fd;

    StreamWFRResult waitForReady2(StreamWFRSet sets, timespec *timeout);
    
    static void waitForReady2(const std::vector<Stream*> *streams, StreamWFRSet sets,
      timespec *timeout, std::vector<StreamWFRResult> *result);

    class StreamException : public Exception {
        public :
          StreamException(void);
          StreamException(int code);
          StreamException(int code, std::string message);
    };

    friend class StreamWFRResult;
  public:
    
    virtual ~Stream(void);
    virtual void closeStream(void);
    
    StreamWFRResult waitForReady(StreamWFRSet sets);
    StreamWFRResult waitForReady(StreamWFRSet sets, uint64_t nanosec);
    StreamWFRResult waitForReady(StreamWFRSet sets, time_t sec, long nanosec);

    static void waitForReady( const std::vector<Stream*> *streams, StreamWFRSet sets,
                              std::vector<StreamWFRResult> *result);
    static void waitForReady( const std::vector<Stream*> *streams, uint64_t nanosec, 
                              StreamWFRSet sets, std::vector<StreamWFRResult> *result);
    static void waitForReady( const std::vector<Stream*> *streams, time_t sec, long nanosec, 
                              StreamWFRSet sets, std::vector<StreamWFRResult> *result);
 
};

class StreamWFRResult {
  public:
    int index;
    StreamWFRSet sets;
    Stream::StreamException e;

    bool setIsNone(void);
    bool setIsRead(void);
    bool setIsWrite(void);
    bool setIsError(void);
    bool setIsAll(void);
    
    StreamWFRResult(int index, StreamWFRSet sets);
    StreamWFRResult(int index, StreamWFRSet sets, int code);
    StreamWFRResult(int index, StreamWFRSet sets, int code, std::string message);
    ~StreamWFRResult(void);
};

#endif
