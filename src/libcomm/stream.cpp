#include "stream.h"
#include "types_utils.h"

#include <errno.h>

#define computeStreamWFRSets(fd, fdsReadPtr, fdsWritePtr, fdsErrorPtr)\
   (StreamWFRSet)\
   (((fdsReadPtr != NULL) ? (FD_ISSET(fd, fdsReadPtr) & 0x1) : 0)\
  | ((fdsWritePtr != NULL) ? (FD_ISSET(fd, fdsWritePtr) << 1 & 0x2) : 0)\
  | ((fdsErrorPtr != NULL) ? (FD_ISSET(fd, fdsErrorPtr) << 2 & 0x3) : 0))

bool StreamWFRResult::setIsNone(void) {
  return (sets == STREAM_WFR_NONE);
}

bool StreamWFRResult::setIsRead(void) {
  return ((sets & STREAM_WFR_READ) != 0);
}

bool StreamWFRResult::setIsWrite(void) {
  return ((sets & STREAM_WFR_WRITE) != 0);
}

bool StreamWFRResult::setIsError(void) {
  return ((sets & STREAM_WFR_ERROR) != 0);
}

bool StreamWFRResult::setIsAll(void) {
  return (sets == STREAM_WFR_ALL);
}

StreamWFRResult::StreamWFRResult(int index, StreamWFRSet sets)
  : index(index), sets(sets) {
}

StreamWFRResult::StreamWFRResult(int index, StreamWFRSet sets, int code)
  : index(index), sets(sets), e(code) {
}

StreamWFRResult::StreamWFRResult(int index, StreamWFRSet sets, int code, std::string message)
  : index(index), sets(sets), e(code, message) {
}

StreamWFRResult::~StreamWFRResult(void) {
}

StreamWFRResult Stream::waitForReady2(StreamWFRSet sets, timespec *timeout) {
  fd_set fdsRead, fdsWrite, fdsError;
  fd_set *fdsReadPtr, *fdsWritePtr, *fdsErrorPtr;
  fdsReadPtr = fdsWritePtr = fdsErrorPtr = NULL;

  int resultSelect;
  int index = 0;
  StreamWFRSet resultSets = STREAM_WFR_NONE;

  FD_ZERO(&fdsRead);
  FD_ZERO(&fdsWrite);
  FD_ZERO(&fdsError);

  if ((sets & STREAM_WFR_READ) != 0) {
    FD_SET(fd, &fdsRead);
    fdsReadPtr = &fdsRead;
  }
  if ((sets & STREAM_WFR_WRITE) != 0) {
    FD_SET(fd, &fdsWrite);
    fdsWritePtr = &fdsWrite;
  }
  if ((sets & STREAM_WFR_ERROR) != 0)  {
    FD_SET(fd, &fdsError);
    fdsErrorPtr = &fdsError;
  }

  resultSelect = pselect(fd+1, fdsReadPtr, fdsWritePtr, fdsErrorPtr, timeout, NULL);
  
  switch (resultSelect) {
    case -1:
      resultSets = computeStreamWFRSets(fd, fdsReadPtr, fdsWritePtr, fdsErrorPtr);
      return StreamWFRResult(index, resultSets, errno);
    case 0:
      index = -1;
      return StreamWFRResult(index, resultSets, EX_STREAM_TIMEOUT, "Timeout on waitForReady.");
    default:
      resultSets = computeStreamWFRSets(fd, fdsReadPtr, fdsWritePtr, fdsErrorPtr);
      return StreamWFRResult(index, resultSets);
  }
}

#include <iostream>
void Stream::waitForReady2(const std::vector<Stream*> *streams, StreamWFRSet sets,
  timespec *timeout, std::vector<StreamWFRResult> *result) {
  
  std::vector<Stream*>::const_iterator iter;
  fd_set fdsRead, fdsWrite, fdsError;
  fd_set *fdsReadPtr, *fdsWritePtr, *fdsErrorPtr;
  fdsReadPtr = fdsWritePtr = fdsErrorPtr = NULL;
  fd_set* nullPtr = NULL;

  int highestFd = 0;
  int resultSelect;

  FD_ZERO(&fdsRead);
  FD_ZERO(&fdsWrite);
  FD_ZERO(&fdsError);

  bool read = (sets & STREAM_WFR_READ) != 0;
  bool write = (sets & STREAM_WFR_WRITE) != 0;
  bool error = (sets & STREAM_WFR_ERROR) != 0;

  if (read | write | error) {
    for (iter = streams->begin(); iter != streams->end(); ++iter) {
      Stream *stream = *iter;
      if (stream != NULL) {
        if (read) {
          FD_SET(stream->fd, &fdsRead);
          fdsReadPtr = &fdsRead;
        }
        if (write) {
          FD_SET(stream->fd, &fdsWrite);
          fdsWritePtr = &fdsWrite;
        }
        if (error)  {
          FD_SET(stream->fd, &fdsError);
          fdsErrorPtr = &fdsError;
        }
        if (highestFd < stream->fd) highestFd = stream->fd;
      }
    }
  }
 
  resultSelect = pselect(highestFd+1, fdsReadPtr, fdsWritePtr, fdsErrorPtr, timeout, NULL);
  
  if (result != NULL) {
    Stream *stream;
    StreamWFRSet resultSets;

    switch (resultSelect) {
      case -1:
        for (size_t i = 0; i<streams->size(); ++i) {
          std::cout << "HERE:" << i << std::endl;
          stream = (*streams)[i];
          if (stream != NULL) {
            resultSets = computeStreamWFRSets(stream->fd, nullPtr, nullPtr, fdsErrorPtr);
            if (resultSets != STREAM_WFR_NONE) {
              result->push_back(StreamWFRResult(i, resultSets, errno));
            } 
          }
        }
        break;
      case 0:
        resultSets = STREAM_WFR_NONE;
        result->push_back(StreamWFRResult(-1, resultSets, EX_STREAM_TIMEOUT,
          "Timeout on waitForReady."));
        break;
      default:
        for (size_t i = 0; i<streams->size(); ++i) {
          stream = (*streams)[i];
          if (stream != NULL) {
            resultSets = computeStreamWFRSets(stream->fd, fdsReadPtr, fdsWritePtr, fdsErrorPtr);
            if (resultSets != STREAM_WFR_NONE) {
              result->push_back(StreamWFRResult(i, resultSets));
            } 
          }
        }
        break;
    }
  }
}

Stream::~Stream(void) {
}

void Stream::closeStream(void) {
  int result;
  
  result = close(fd);
  if (result == -1) {
    throw Stream::StreamException(errno); 
  }
}

StreamWFRResult Stream::waitForReady(StreamWFRSet sets) {
  return waitForReady2(sets, NULL);
}

StreamWFRResult Stream::waitForReady(StreamWFRSet sets, uint64_t nanosec) {
  timespec ts;
  nanosecToSecNsec(nanosec, &(ts.tv_sec), &(ts.tv_nsec));
  return waitForReady2(sets, &ts);
}

StreamWFRResult Stream::waitForReady(StreamWFRSet sets, time_t sec, long nanosec) {
  timespec ts = {sec, nanosec};
  return waitForReady2(sets, &ts);
}

void Stream::waitForReady(const std::vector<Stream*> *streams, StreamWFRSet sets,
  std::vector<StreamWFRResult> *result) {
  waitForReady2(streams, sets, NULL, result);
}

void Stream::waitForReady(const std::vector<Stream*> *streams, uint64_t nanosec, 
  StreamWFRSet sets, std::vector<StreamWFRResult> *result) {
  timespec ts;
  nanosecToSecNsec(nanosec, &(ts.tv_sec), &(ts.tv_nsec));
  waitForReady2(streams, sets, &ts, result);
}

void Stream::waitForReady(const std::vector<Stream*> *streams, time_t sec, long nanosec, 
  StreamWFRSet sets, std::vector<StreamWFRResult> *result) {
  timespec ts = {sec, nanosec};
  waitForReady2(streams, sets, &ts, result);
}

Stream::StreamException::StreamException(void) {}

Stream::StreamException::StreamException(int code):
  Exception(code) {
}

Stream::StreamException::StreamException(int code, std::string message):
  Exception(code, message) {
}
