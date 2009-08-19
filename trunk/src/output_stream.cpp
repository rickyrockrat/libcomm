#include "output_stream.h"

#include "net_message.h"
#include "serialization_manager.h"

#define DEFAULT_WRITE_BUFFER_SIZE 4096

OutputStream::~OutputStream(void) {
}

char *OutputStream::generateRemainingData(  const struct iovec *iov, int iovcnt,
                              size_t totalWritten, size_t *toWrite) {
  size_t currentByteCount = 0;
  size_t currentCopyByteCount = 0;
  int i;
  char *data = NULL;

  for (i = 0; i<iovcnt; ++i) {
    size_t len = iov[i].iov_len;
    if ((currentByteCount + len) > totalWritten) {
      size_t toCopy = (currentByteCount+len) - totalWritten;
      
      data = (char*) realloc(data, toCopy); 
      memcpy(&(data[currentCopyByteCount]), &(((char*)(iov[i].iov_base))[len-toCopy]), toCopy); 
      currentCopyByteCount += toCopy;
    }
    currentByteCount += len;
  }

  *toWrite = currentByteCount - totalWritten;
  
  return data;
}

ssize_t OutputStream::writeObject2(const Serializable &object, const NetAddress *addr) {
  NetMessage *message;
  SerializationManager *serManager;
  struct iovec *iov;
  int iovcnt;
  ssize_t sizeWritten;
  
  serManager = SerializationManager::getSerializationManager();
  message = serManager->serialize(object, NULL);
  iov = (struct iovec*) message->getData(&iovcnt);
  
  try {
    sizeWritten = writeData(iov, iovcnt, addr);
    free(iov);
    delete message;
  } catch (Exception &e) {
    free(iov);
    delete message;
    throw e;
  }

  return sizeWritten;
}

ssize_t OutputStream::writeString2(const std::string &string, const NetAddress *addr) {
  std::string toSend = string;
  toSend += "\r\n";
  
  return writeData(toSend.c_str(), toSend.length(), 0, addr); 
}

ssize_t OutputStream::writeBytes2(const Buffer<char> &data, int flags, const NetAddress *addr) {
  return writeData(data.data(), data.size(), flags, addr); 
}

ssize_t OutputStream::writeObject(const Serializable &object) {
  return writeObject2(object, NULL);
}

ssize_t OutputStream::writeString(const std::string &string) {
  return writeString2(string, NULL);
}

ssize_t OutputStream::writeBytes(const Buffer<char> &data, int flags) {
  return writeBytes2(data, flags, NULL);
}


OutputStream::OutputStreamException::OutputStreamException(int code, size_t sizeToWrite)
  : Exception(code), sizeToWrite(sizeToWrite) {
    data = NULL;
    sizeWritten = 0;
}

OutputStream::OutputStreamException::OutputStreamException(int code, size_t sizeToWrite, std::string message)
  : Exception(code, message), sizeToWrite(sizeToWrite), sizeWritten(0), data(NULL) {
}

OutputStream::OutputStreamException::OutputStreamException(int code,
  size_t sizeToWrite, char *data, ssize_t sizeWritten)
  : Exception(code), sizeToWrite(sizeToWrite), sizeWritten(sizeWritten),
    data(data) {
}

OutputStream::OutputStreamException::~OutputStreamException(void) {
  if (data != NULL) {
    free(data);
  }
}

size_t OutputStream::OutputStreamException::getSizeToWrite(void) {
  return sizeToWrite;
}

ssize_t OutputStream::OutputStreamException::getSizeWritten(void) {
  return sizeWritten;
}

const char *OutputStream::OutputStreamException::getNonWrittenData(void) {
  return data;
}

BufferedOutputStream::BufferedOutputStream(void): buffers(NULL), buffersCount(0),
  dataSize(0), lastNetAddress(NULL), writeBufferSize(DEFAULT_WRITE_BUFFER_SIZE) {
}

BufferedOutputStream::~BufferedOutputStream(void) {
  clearBuffers();
}

void BufferedOutputStream::closeStream(void) {
  flushBuffers();
}

void BufferedOutputStream::clearBuffers(void) {
  if (buffers != NULL) {
    for (int i = 0; i<buffersCount; ++i) {
      free(buffers[i].iov_base);
    }
    free(buffers);

    buffers = NULL;
    dataSize = 0;
    buffersCount = 0;
  }

  if (lastNetAddress != NULL) {
    delete lastNetAddress;
    lastNetAddress = NULL;
  }
}

ssize_t BufferedOutputStream::writeData(  const char *data, size_t size, int flags,
                                          const NetAddress *addr) {
  ++buffersCount;
  dataSize += size;
  buffers = (struct iovec*) realloc(buffers, buffersCount*sizeof(struct iovec));
  buffers[buffersCount-1].iov_base = malloc(size);
  memcpy(buffers[buffersCount-1].iov_base, data, size);
  buffers[buffersCount-1].iov_len = size;

  if (addr != NULL) {
    if (lastNetAddress != NULL) delete lastNetAddress;
    lastNetAddress = new NetAddress(*addr);
  }

  if (dataSize > writeBufferSize) {
    flushBuffers();
  }
}

ssize_t BufferedOutputStream::writeData(  const struct iovec *iov, int iovCount,
                    const NetAddress *addr) {
  int oldBuffersCount = buffersCount;

  buffersCount += iovCount;
  buffers = (struct iovec*) realloc(buffers, iovCount*sizeof(struct iovec));
  
  for(int i = 0; i<iovCount; ++i) {
    struct iovec &ciov = buffers[oldBuffersCount+i];
    ciov.iov_base = malloc(iov[i].iov_len);
    memcpy(ciov.iov_base, iov[i].iov_base, iov[i].iov_len);
    ciov.iov_len = iov[i].iov_len;
    dataSize += ciov.iov_len;
  }

  if (addr != NULL) {
    if (lastNetAddress != NULL) delete lastNetAddress;
    lastNetAddress = new NetAddress(*addr);
  }

  if (dataSize > writeBufferSize) {
    flushBuffers();
  }
}

ssize_t BufferedOutputStream::flushBuffers(void) {
  ssize_t dataWritten = writeRawData(buffers, buffersCount,
    lastNetAddress);
  clearBuffers();
  return dataWritten;
}

size_t BufferedOutputStream::getWriteBufferSize(void) const {
  return writeBufferSize;
}

void BufferedOutputStream::setWriteBufferSize(size_t bs) {
  writeBufferSize = bs;
}
