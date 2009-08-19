#include "input_stream.h"
#include "net_message.h"

#define DEFAULT_READ_BUFFER_SIZE 1500
#define MAX_BUFFER_SIZE 15000


InputStream::InputStream(void): readBufferSize(DEFAULT_READ_BUFFER_SIZE) {}

InputStream::~InputStream(void) {};

size_t InputStream::getReadBufferSize(void) {
  return readBufferSize;
}

void InputStream::setReadBufferSize(size_t bs) {
  readBufferSize = bs;
}

char *InputStream::readDataForced(size_t size, NetAddress *addr) {
  char *buffer;
  ssize_t sizeRead = 0;
  size_t totalRead = 0;
  
  buffer = (char*) malloc(size);
  while (totalRead != size) {
    sizeRead = readData(&(buffer[totalRead]), size-totalRead, 0, addr);
    totalRead += sizeRead;
  }

  return buffer;
}

void InputStream::readDataForced(char * buffer, size_t size, NetAddress *addr) {
  ssize_t sizeRead = 0;
  size_t totalRead = 0;
  
  while (totalRead != size) {
    sizeRead = readData(&(buffer[sizeRead]), size-totalRead, 0, addr);
    totalRead += sizeRead;
  }
}


bool InputStream::containsCRLF(const char *buff, off_t start, off_t end, size_t *index) {
  off_t i;

  for (i = start; i < end; ++i) {
    if (buff[i] == '\r') {
      if (i < (end-1)) {
        if (buff[i+1] == '\n') {
          *index = i+2;
          return true;
        }
      }
    }
  }
  return false;
}

String *InputStream::tryBuildString(const char *data, off_t startCheck, size_t size, size_t *index) {
  String *returnString = NULL;

  if (containsCRLF(data, startCheck, size, index)) {
    returnString = new String(data, *index-2); 
  }

  return returnString;
}


void InputStream::parseNetMessageContent(NetMessage *netMessage, size_t netMessageSize) {
  size_t blockSize;
  size_t headerSize;
  size_t totalSize = 0;
  bool toFree;
  NetMessage *childNetMessage;
  char *blockData;

  
  while (totalSize < netMessageSize) {
    childNetMessage = parseBlockHeader(netMessage, &blockSize, &headerSize, &toFree);
    if (childNetMessage != NULL) {
      // NetMessage
      netMessage->addMessage(childNetMessage); 
      if (blockSize != 0) {
        parseNetMessageContent(childNetMessage, blockSize);
      }
    } else {
      // BlockData
      blockData = readDataForced(blockSize, NULL);
      netMessage->addDataBlock(blockData, blockSize,toFree, false);
    }
    totalSize += (blockSize + headerSize);
  }
}

NetMessage *InputStream::parseBlockHeader(NetMessage *netMessage, size_t *blockSize,
  size_t *headerSize, bool *toFree, NetAddress *addr) {
  size_t nextSize;
  NetMessage *newNetMessage = NULL;
  char buff[32];

  //flags
  try {
    size_t sizeFlags = NetMessage::getFlagsHeaderSize();
    readDataForced(buff, sizeFlags, addr); 
    newNetMessage = netMessage->parseFlags(buff, &nextSize, toFree);
    if (headerSize != NULL) *headerSize = sizeFlags + nextSize;

    readDataForced(buff, nextSize, NULL); 
    if (newNetMessage == NULL) {
      //Block
      netMessage->parseTypeSize(buff, false, nextSize, blockSize);
    } else {
      //NetMessage
      newNetMessage->parseTypeSize(buff, true, nextSize, blockSize);
    }
  } catch (Exception &e) {
    if (newNetMessage != NULL) {
      delete newNetMessage;
    }
    throw e;
  }

  return newNetMessage;
}


Buffer<char> *InputStream::readBytes2(Buffer<char> *buff, int flags, NetAddress *addr) {
  size_t size;
  ssize_t sizeRead;
  bool mustAllocate = (buff == NULL);
  
  if (mustAllocate) {
    size = readBufferSize;
    buff = new Buffer<char>(size);
  } else {
    size = buff->size();
  }
  
  try {
    sizeRead = readData(buff->array, size, flags, addr);
    if ((size_t) sizeRead < size) buff->resize(sizeRead);
  } catch (Exception &e) {
    if (mustAllocate) delete buff;
    throw e;
  }

  return buff;
}


String *InputStream::readString2(NetAddress *addr) {
  char *buff = NULL;
  size_t endIndex = 0;
  size_t sizeBuff = 0;
  size_t totalRead = 0;
  ssize_t sizeRead = 0;
  String *str = NULL;

  while (str == NULL) {
    sizeBuff += readBufferSize;
    buff = (char*) realloc(buff, sizeBuff);

    try {
      sizeRead = peekData(buff, sizeBuff, addr);
      str = tryBuildString(buff, totalRead, totalRead + sizeRead, &endIndex);
      totalRead += sizeRead;
    } catch (Exception &e) {
      free(buff);
      throw e;
    }
  }

  readData(buff, endIndex, 0, addr);
  free(buff);

  return str;
}

Serializable *InputStream::readObject2(NetAddress *addr) {
  size_t netMessageSize;
  NetMessage *netMessage;
  SerializationManager *serManager;
  void *object = NULL;

  netMessage = new NetMessage();
  try {
    parseBlockHeader(netMessage, &netMessageSize, NULL, NULL, addr);
    parseNetMessageContent(netMessage, netMessageSize);

  } catch (Exception &e) {
    netMessage->deleteAllData();
    delete netMessage;
    throw e;
  }

  serManager = SerializationManager::getSerializationManager();
  object = serManager->deserialize(*netMessage, false);

  if (object == NULL) netMessage->deleteAllData();
  delete netMessage;
  
  return (Serializable*) object;
}


Buffer<char> *InputStream::readBytes(Buffer<char> *buff, int flags) {
  return readBytes2(buff, flags, NULL);
}

Buffer<char> *InputStream::readBytes(Buffer<char> *buff, uint64_t nanosec, int flags) {
  time_t sec;
  long nsec;

  nanosecToSecNsec(nanosec, &sec, &nsec);
  return readBytes(buff, sec, nsec, flags);
}


Buffer<char> *InputStream::readBytes(Buffer<char> *buff, time_t sec, long nanosec, int flags) {
  StreamWFRResult waitResult =
    waitForReady((StreamWFRSet) (STREAM_WFR_READ | STREAM_WFR_ERROR), sec, nanosec);
  // timeout || error
  if ((waitResult.setIsNone()) || (waitResult.setIsError())) {
    throw waitResult.e;;
  } else {
    return readBytes(buff, flags);
  }
}

String *InputStream::readString(void) {
  return readString2(NULL);
}

String *InputStream::readString(uint64_t nanosec) {
  time_t sec;
  long nsec;

  nanosecToSecNsec(nanosec, &sec, &nsec);
  return readString(sec, nsec);
}

String *InputStream::readString(time_t sec, long nanosec) {
  StreamWFRResult waitResult =
    waitForReady((StreamWFRSet) (STREAM_WFR_READ | STREAM_WFR_ERROR), sec, nanosec);
  // timeout || error
  if ((waitResult.setIsNone()) || (waitResult.setIsError())) {
    throw waitResult.e;;
  } else {
    return readString();
  }
}

Serializable *InputStream::readObject(void) {
  return readObject2(NULL);
}

Serializable *InputStream::readObject(uint64_t nanosec) {
  time_t sec;
  long nsec;

  nanosecToSecNsec(nanosec, &sec, &nsec);
  return readObject(sec, nsec);
}

Serializable *InputStream::readObject(time_t sec, long nanosec) {
  StreamWFRResult waitResult = 
    waitForReady((StreamWFRSet) (STREAM_WFR_READ | STREAM_WFR_ERROR), sec, nanosec);
  // timeout || error
  if ((waitResult.setIsNone()) || (waitResult.setIsError())) {
    throw waitResult.e;;
  } else {
    return readObject();
  }
}

InputStream::InputStreamException::InputStreamException(int code)
  : Exception(code) {}

InputStream::InputStreamException::InputStreamException(int code, std::string message)
  : Exception(code, message) {}




BufferedInputStream::BufferedInputStream(void) : buf(NULL), bufferSize(0),
  offset(0) {}

BufferedInputStream::~BufferedInputStream(void) {
  if (buf != NULL) free(buf);
}

size_t InputStreamInterface::getReadBufferSize(void) {
  throw InputStream::InputStreamException(EX_ISTREAM_VIRTUAL_CALL,
    "InputStreamInterface::getReadBufferSize shall not be called directly");}
void InputStreamInterface::setReadBufferSize(size_t bs) {
  throw InputStream::InputStreamException(EX_ISTREAM_VIRTUAL_CALL,
    "InputStreamInterface::setReadBufferSize shall not be called directly");}
Buffer<char> *InputStreamInterface::readBytes(Buffer<char> *buff, int flags) {
  throw InputStream::InputStreamException(EX_ISTREAM_VIRTUAL_CALL,
    "InputStreamInterface::readBytes shall not be called directly");}
Buffer<char> *InputStreamInterface::readBytes(Buffer<char> *buff, uint64_t nanosec, int flags) {
  throw InputStream::InputStreamException(EX_ISTREAM_VIRTUAL_CALL,
    "InputStreamInterface::readBytes shall not be called directly");}
Buffer<char> *InputStreamInterface::readBytes(Buffer<char> *buff, time_t sec, long nanosec, int flags) {
  throw InputStream::InputStreamException(EX_ISTREAM_VIRTUAL_CALL,
    "InputStreamInterface::readBytes shall not be called directly");}
String *InputStreamInterface::readString(void) {
  throw InputStream::InputStreamException(EX_ISTREAM_VIRTUAL_CALL,
    "InputStreamInterface::readString shall not be called directly");}
String *InputStreamInterface::readString(uint64_t nanosec) {
  throw InputStream::InputStreamException(EX_ISTREAM_VIRTUAL_CALL,
    "InputStreamInterface::readString shall not be called directly");}
String *InputStreamInterface::readString(time_t sec, long nanosec) {
  throw InputStream::InputStreamException(EX_ISTREAM_VIRTUAL_CALL,
    "InputStreamInterface::readString shall not be called directly");}
Serializable *InputStreamInterface::readObject(void) {
  throw InputStream::InputStreamException(EX_ISTREAM_VIRTUAL_CALL,
    "InputStreamInterface::readObject shall not be called directly");}
Serializable *InputStreamInterface::readObject(uint64_t nanosec) {
  throw InputStream::InputStreamException(EX_ISTREAM_VIRTUAL_CALL,
    "InputStreamInterface::readObject shall not be called directly");}
Serializable *InputStreamInterface::readObject(time_t sec, long nanosec){
  throw InputStream::InputStreamException(EX_ISTREAM_VIRTUAL_CALL,
    "InputStreamInterface::readObject shall not be called directly");}


void BufferedInputStream::fillBuffer(size_t size, int flags, bool netAddress) {
  ssize_t readBytes;
  size_t toAllocateMore = ((size / readBufferSize) 
    + (((size % readBufferSize) != 0) ? 1 : 0)) * readBufferSize;

  buf = (char*) realloc(buf, bufferSize + toAllocateMore);
  readBytes = readRawData(&(buf[bufferSize]), toAllocateMore, flags, (netAddress) ? &lastNetAddress : NULL);
  
  bufferSize += readBytes;
  if ((size_t) readBytes < toAllocateMore) buf = (char*) realloc(buf, bufferSize);
}

void BufferedInputStream::clearBuffer(void) {
  size_t sizeData = bufferSize-offset;
  if (sizeData == 0){
    if (buf != NULL) free(buf);
    buf = NULL;
    bufferSize = offset = 0;
  } else {
    if ((sizeData != bufferSize) && (bufferSize > MAX_BUFFER_SIZE)) {
      char *bufTemp = (char*) malloc(sizeData);
      
      memcpy(bufTemp, &(buf[offset]), sizeData);
      free(buf);
      buf = bufTemp;
      offset = 0;
      bufferSize = sizeData;
    }
  }
}

ssize_t BufferedInputStream::readData(  char *buffer, size_t size, int flags,
                                        NetAddress *addr) {
  size_t sizeToCopy;

  if ((bufferSize - offset) < size) fillBuffer((size-(bufferSize-offset)), flags, (addr != NULL));
  sizeToCopy = ((bufferSize - offset) < size) ? (bufferSize - offset) : size;

  memcpy(buffer, &(buf[offset]), sizeToCopy);
  offset += sizeToCopy;
  if (addr != NULL) *addr = lastNetAddress;
  
  clearBuffer();
  return sizeToCopy;
}


ssize_t BufferedInputStream::peekData(  char *buffer, size_t size, NetAddress *addr) {
  size_t sizeToCopy;

  if ((bufferSize - offset) < size) fillBuffer((size-(bufferSize-offset)), 0, (addr != NULL));
  sizeToCopy = ((bufferSize - offset) < size) ? (bufferSize - offset) : size;

  memcpy(buffer, &(buf[offset]), sizeToCopy);
  if (addr != NULL) *addr = lastNetAddress;
  
  return sizeToCopy;
}
