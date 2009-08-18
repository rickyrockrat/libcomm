#include "output_stream.h"

#include "net_message.h"
#include "serialization_manager.h"


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
