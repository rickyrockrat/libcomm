#ifndef INPUT_STREAM_H
#define INPUT_STREAM_H

#include "stream.h"
#include "net_address.h"
#include "serializable.h"
#include "structs/string_serializable.h"
#include "structs/buffer_serializable.h"

class NetMessage;
class InputStreamInterface;

class InputStream: virtual public Stream {
  protected:
    size_t readBufferSize;

    InputStream(void);

    virtual ssize_t readData(   char *buffer, size_t size, int flags,
                                NetAddress *addr) = 0;
    virtual ssize_t peekData(   char *buffer, size_t size,
                                NetAddress *addr) = 0;
    
    char *readDataForced(size_t size, NetAddress *addr);
    void readDataForced(char *buffer, size_t size, NetAddress *addr);

    bool containsCRLF(const char *buff, off_t start, off_t end, size_t *index);
    String *tryBuildString(const char *data, off_t startCheck, size_t size, size_t *index);

    void parseNetMessageContent(NetMessage *parent, size_t netMessageSize);
    NetMessage *parseBlockHeader( NetMessage *netMessage, size_t *blockSize,
                                  size_t *headerSize, bool *toFree, NetAddress *addr = NULL);

    Buffer<char> *readBytes2(Buffer<char> *buff, int flags, NetAddress *addr);
    String *readString2(NetAddress *addr);
    Serializable *readObject2(NetAddress *addr);

    friend class InputStreamInterface;
     
  public:
    
    virtual ~InputStream(void);

    size_t getReadBufferSize(void);
    void setReadBufferSize(size_t bs);

    Buffer<char> *readBytes(Buffer<char> *buff, int flags = 0);
    Buffer<char> *readBytes(Buffer<char> *buff, uint64_t nanosec, int flags = 0);
    Buffer<char> *readBytes(Buffer<char> *buff, time_t sec, long nanosec, int flags = 0);
    String *readString(void);
    String *readString(uint64_t nanosec);
    String *readString(time_t sec, long nanosec);
    Serializable *readObject(void);
    Serializable *readObject(uint64_t nanosec);
    Serializable *readObject(time_t sec, long nanosec);

    class InputStreamException : public Exception {
      public :
        InputStreamException(int code);
        InputStreamException(int code, std::string message);
    };


};

class InputStreamInterface {
  public:
    virtual size_t getReadBufferSize(void);
    virtual void setReadBufferSize(size_t bs);

    virtual Buffer<char> *readBytes(Buffer<char> *buff, int flags = 0);
    virtual Buffer<char> *readBytes(Buffer<char> *buff, uint64_t nanosec, int flags = 0);
    virtual Buffer<char> *readBytes(Buffer<char> *buff, time_t sec, long nanosec, int flags = 0);
    virtual String *readString(void);
    virtual String *readString(uint64_t nanosec);
    virtual String *readString(time_t sec, long nanosec);
    virtual Serializable *readObject(void);
    virtual Serializable *readObject(uint64_t nanosec);
    virtual Serializable *readObject(time_t sec, long nanosec);
};

class BufferedInputStream: public InputStream{
  private:
    char *buf;
    size_t bufferSize;
    size_t offset;
    NetAddress lastNetAddress;

    ssize_t readData(   char *buffer, size_t size, int flags,
                        NetAddress *addr);
    ssize_t peekData(   char *buffer, size_t size,
                        NetAddress *addr);

    void fillBuffer(size_t size, int flags, bool netAddress);
    void clearBuffer(void);

  protected:
    BufferedInputStream(void);
    virtual ~BufferedInputStream(void);

    virtual ssize_t readRawData(   char *buffer, size_t size, int flags,
                                   NetAddress *addr) = 0;
};

#endif
