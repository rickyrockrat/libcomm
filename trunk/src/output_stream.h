#ifndef OUTPUT_STREAM_H
#define OUTPUT_STREAM_H

#include "stream.h"
#include "net_address.h"
#include "serializable.h"
#include "structs/string_serializable.h"
#include "structs/buffer_serializable.h"


class OutputStream: virtual public Stream {
  protected:
    virtual ssize_t writeData(  const char *data, size_t size, int flags,
                                const NetAddress *addr) = 0;
    virtual ssize_t writeData(  const struct iovec *iov, int iovcnt,
                                const NetAddress *addr) = 0;

     
    char *generateRemainingData(  const struct iovec *iov, int iovcnt,
                                  size_t totalWritten, size_t *toWrite);

    class OutputStreamException : public Exception {
      private:
        size_t sizeToWrite;
        ssize_t sizeWritten;
        
        char *data;
      public :
        OutputStreamException(  int code, size_t sizeToWrite);
        OutputStreamException(  int code, size_t sizeToWrite, std::string message);
        OutputStreamException(  int code, size_t sizeToWrite, char *data,
                                ssize_t sizeWritten);
        ~OutputStreamException(void);

        size_t getSizeToWrite(void);
        ssize_t getSizeWritten(void);
        const char *getNonWrittenData(void);
    };
    
    ssize_t writeObject2(const Serializable &object, const NetAddress *addr);
    ssize_t writeString2(const std::string &string, const NetAddress *addr);
    ssize_t writeBytes2(const Buffer<char> &data, int flags, const NetAddress *addr);
  
  public:
    
    virtual ~OutputStream();

    ssize_t writeObject(const Serializable &object);
    ssize_t writeString(const std::string &string);
    ssize_t writeBytes(const Buffer<char> &data, int flags = 0);
};

class BufferedOutputStream: public OutputStream {
  private:
    struct iovec *buffers;
    int buffersCount;
    size_t dataSize;
    NetAddress *lastNetAddress;
    size_t writeBufferSize;

    void clearBuffers(void);
    
    ssize_t writeData(  const char *data, size_t size, int flags,
                        const NetAddress *addr);
    ssize_t writeData(  const struct iovec *iov, int iovcnt,
                        const NetAddress *addr);

  protected:
     virtual ssize_t writeRawData(  const struct iovec *iov, int iovcnt,
                                    const NetAddress *addr) = 0;

  public:
    BufferedOutputStream(void);
    virtual ~BufferedOutputStream(void);
    
    void closeStream(void);
    ssize_t flushBuffers(void);
    size_t getWriteBufferSize(void) const;
    void setWriteBufferSize(size_t bs);
};

#endif
