#include <time.h>
#include <sstream>

#include "net_message.h"
#include "types_utils.h"
#include "exception.h"

#define FAST_VECT_BLOCK_SIZE 8

const size_t flagsHeader = sizeof(uint8_t);
const size_t typeHeader = sizeof(uint16_t);

#define flagsTypeHeaders (flagsHeader + typeHeader)

#include <iostream>

NetMessage::NetMessage(void) {
  initVars();
  addIovec(1);
  freeIov[0] = true;
}

NetMessage::NetMessage(uint16_t type): type(type)  {
  initVars();
  addIovec(1);
  freeIov[0] = true;
}

NetMessage::NetMessage(uint16_t type, char *value, size_t primitiveSize): type(type) {
  initVars();
  addIovec(1);
  freeIov[0] = true;
  this->primitiveValue = value;
  this->primitiveSize = primitiveSize;
}

NetMessage::NetMessage(const NetMessage &net_message) {
  std::vector<NetMessage*>::const_iterator iter;

  initVars();
  type = net_message.type;
  dataSize = net_message.dataSize;

  if (net_message.primitiveSize != 0) {
    primitiveSize = net_message.primitiveSize;
    primitiveValue = (char*) malloc(primitiveSize);
    memcpy(primitiveValue, net_message.primitiveValue, primitiveSize);
  } else {
    addIovec(net_message.iovCount);
    
    for (int i = 0; i<iovCount; ++i) {
      size_t size = net_message.data[i].iov_len;
      data[i].iov_len = size;
      data[i].iov_base = (char*) malloc(size);
      memcpy(data[i].iov_base, net_message.data[i].iov_base, size);
      freeIov[i] = net_message.freeIov[i];
      //std::cout << freeIov[i] << std::endl;
    }

    nestedNetMessages.reserve(net_message.nestedNetMessages.size());  
    for ( iter = net_message.nestedNetMessages.begin(); 
          iter != net_message.nestedNetMessages.end();
          ++iter) {
      nestedNetMessages.push_back(new NetMessage(**iter));
    }
  }
}

NetMessage::~NetMessage() {
  std::vector<NetMessage*>::iterator iter;
  
  if (primitiveValue != NULL) free(primitiveValue);
  for (int i = 0; i<iovCount; ++i) {
    if (freeIov[i]) {
      free(data[i].iov_base);
    }
  }
  free(data);
  free(freeIov);

  for (iter = nestedNetMessages.begin(); iter < nestedNetMessages.end(); ++iter) {
    delete *iter;
  }
  nestedNetMessages.clear();
}

void NetMessage::initVars(void) {
  primitiveValue = NULL;
  data = NULL;
  freeIov = NULL;
  iovCount = iovAllocated = primitiveSize = dataSize = 0;
  headersGenerated = false;
}

void NetMessage::addIovec(size_t iovcnt) {
  int iovToAllocate;
  int oldIovAllocated;
  int newIovAllocated;
  iovCount += iovcnt;
  
  iovToAllocate = iovCount - iovAllocated;

  if (iovToAllocate > 0) {
    int nbBlockToAllocate = (iovToAllocate / FAST_VECT_BLOCK_SIZE) 
      + (iovToAllocate % FAST_VECT_BLOCK_SIZE != 0) ? 1 : 0;

    oldIovAllocated = iovAllocated;
    newIovAllocated = nbBlockToAllocate * FAST_VECT_BLOCK_SIZE;
    iovAllocated += newIovAllocated;
    data = (chariovec*) realloc(data, iovAllocated * sizeof(chariovec));
    freeIov = (bool*) realloc(freeIov, iovAllocated * sizeof(bool));
    //ptrIov = (size_t*) realloc(ptrIov, iovAllocated * sizeof(size_t));
    memset(&(data[oldIovAllocated]), 0, newIovAllocated*sizeof(chariovec));
    memset(&(freeIov[oldIovAllocated]), 0, newIovAllocated*sizeof(bool));
    //memset(&(ptrIov[oldIovAllocated]), 0, newIovAllocated*sizeof(size_t));
  }
}

size_t NetMessage::generateBlockHeader(int type, size_t size, bool toFree, chariovec &iov) {
  size_t realSize;
  size_t sizeHeaders;
  int index = 0;
  swap_64 swap64;

  realSize = findRealSize(size);
  sizeHeaders = ((type < 0) ? flagsHeader : flagsTypeHeaders) + realSize;
  iov.iov_len = sizeHeaders;
  iov.iov_base = (char*) realloc(iov.iov_base, sizeHeaders);

  // Flags
  iov.iov_base[index] = 0;
  iov.iov_base[index] |= (realSize & 0x1F);
  iov.iov_base[index] |= ((toFree << 6) & 0x40);
  iov.iov_base[index] |= ((((type < 0) ? 0 : 1) << 5) & 0x20);
  index += flagsHeader;

  //Type
  if (type > -1) {
    uint16_t typeCast = type;
    convertToChars(typeCast, &(iov.iov_base[index]));
    index += typeHeader;
  }
  
  // Size

  //std::cout << "GenerateblockSize:" << size << std::endl;
  //std::cout << "RealSize:" << realSize << std::endl;

  swap64.uint = size;
  /*for (int i = 0; i<8; ++i) {
    std::cout << (void*) swap64.chars[i] << " ";
  }
  std::cout << std::endl;*/
  if (isLittleEndian()) {
    for (size_t i = realSize; i > 0; --i) {
      iov.iov_base[index+i-1] = swap64.chars[realSize-i];
    }
  } else {
    memcpy(&(iov.iov_base[index]), &(swap64.chars[8-realSize]), realSize);
  }
  /*std::cout << (void*) iov.iov_base[index] << std::endl;
  std::cout << (void*) iov.iov_base[index+1] << std::endl;
  std::cout << (void*) iov.iov_base[index+2] << std::endl;
  std::cout << (void*) iov.iov_base[index+3] << std::endl;*/

  return sizeHeaders;
}
    
size_t NetMessage::generatePrimitiveBlockHeader(chariovec &iov) {
  int index = 0;
  int sizeHeaders;
  
  sizeHeaders = flagsHeader + primitiveSize;
  iov.iov_len = sizeHeaders;
  iov.iov_base = (char*) realloc(iov.iov_base, sizeHeaders);

  // Flags
  iov.iov_base[index] = 0;
  iov.iov_base[index] |= (type  & 0x1F);
  iov.iov_base[index] |= ((true << 6) & 0x40);
  iov.iov_base[index] |= ((true << 5) & 0x22);
  index += flagsHeader;

  //Primitive value
  memcpy(&(iov.iov_base[index]), primitiveValue, primitiveSize);

  return sizeHeaders;
}

void NetMessage::generateNetMessageHeader(void) {
  if (primitiveValue == NULL) {
    generateBlockHeader((int) type, dataSize, false, data[0]);
  } else {
    generatePrimitiveBlockHeader(data[0]);
  }
  headersGenerated = true;
}

void NetMessage::collectIov(chariovec **iov, int *size) {
  int newSize = *size + iovCount;
  *iov = (chariovec*) realloc(*iov, newSize * sizeof(chariovec));
  std::vector<NetMessage*>::iterator iter;
  
  for (int i = 0; i<iovCount; ++i) {
    (*iov)[*size+i] = data[i];
  }

  *size = newSize;
  
  for (iter = nestedNetMessages.begin(); iter != nestedNetMessages.end(); ++iter) {
    (*iter)->collectIov(iov, size);
  }
}

size_t NetMessage::findRealSize(size_t var) {
  size_t sizeSizeT = sizeof(size_t);
  size_t i = 0;

  while (( i < sizeSizeT) && (var != 0)) {
    var = var >> 8;
    ++i;
  }

  return i;
}

uint16_t NetMessage::getType(void) const {
  return type;
}

uint32_t NetMessage::getDataSize(bool withHeaders){
  if (withHeaders) {
    if (!headersGenerated) {
      generateNetMessageHeader();
    }
    return dataSize + data[0].iov_len;
  } else {
    return dataSize;
  }
}

chariovec *NetMessage::getData(int *iovcnt){
  chariovec *iov = NULL;
   
  if (!headersGenerated) {
    generateNetMessageHeader();
  }
  *iovcnt = 0;
  collectIov(&iov, iovcnt);
  return iov;
}

chariovec *NetMessage::getDataBlocks(int *iovcnt) const {
  //std::cout << "IovCount:" << iovCount << std::endl;
  int iovcntVar = (iovCount-1) / 2;
  chariovec *iov;
  chariovec *from = &(data[2]);

  iov = (chariovec*) malloc(iovcntVar*sizeof(chariovec));

  for (int i = 0; i<iovcntVar; ++i) {
    iov[i] = from[2*i];
  }
  
  if (iovcnt != NULL) *iovcnt = iovcntVar;
  return iov;
}


const std::vector<NetMessage*> &NetMessage::getMessages(void) const {
  return nestedNetMessages;
}


const char *NetMessage::getPrimitiveValue(void) const {
  return primitiveValue;
}

void NetMessage::addMessage(NetMessage *message) {
  dataSize += message->getDataSize(true);
  
  if (nestedNetMessages.capacity() == nestedNetMessages.size()) {
    nestedNetMessages.reserve(nestedNetMessages.size() + FAST_VECT_BLOCK_SIZE);
  }

  nestedNetMessages.push_back(message);
  
  headersGenerated = false;
}

int NetMessage::addDataBlock(char *newData, size_t size, bool toFree,
  bool shouldgenerateBlockHeader) {
  int oldIovCount;
  
  oldIovCount = iovCount;
  if (shouldgenerateBlockHeader) {
    size_t sizeHeaders;

    addIovec(2);
    sizeHeaders = generateBlockHeader(-1, size, toFree, data[oldIovCount]);
    dataSize += (size + sizeHeaders);
    freeIov[oldIovCount] = true;
    ++oldIovCount;
  } else {
    addIovec(1);
    dataSize += size;
  }

  data[oldIovCount].iov_base = newData;
  data[oldIovCount].iov_len  = size;
  freeIov[oldIovCount] = toFree;
  
  headersGenerated = false;

  return oldIovCount;
}

void NetMessage::appendDataToBlock(int blockId, const char *newData, size_t size) {
  chariovec &iov = data[blockId];
  size_t oldSize = iov.iov_len;
  
  dataSize += size;
  iov.iov_len += size;
  iov.iov_base = (char*) realloc(iov.iov_base, iov.iov_len);
  
  memcpy(&(iov.iov_base[oldSize]), newData, size);

  headersGenerated = false;
}


const char *NetMessage::getDataFromBlock(int blockId, size_t &offset, size_t size) const {
  chariovec &iov = data[blockId];
  size_t oldOffset = offset;

  offset += size;
  if (offset == iov.iov_len) offset = 0;
  return &(iov.iov_base[oldOffset]);
}


bool NetMessage::hasDataBlock(void) {
  return (iovCount > 1);
}

/*void NetMessage::setBlockAutoFree(int blockId, bool toFree) {
  freeIov[(blockId*2)+1] = toFree;
}*/

size_t NetMessage::getFlagsHeaderSize(void) {
  return flagsHeader;
}

NetMessage *NetMessage::parseFlags(const char *buff, size_t *nextSize,
  bool *toFree) {
  NetMessage *netMessage = NULL;
  NetMessage *returnNetMessage = NULL;
  bool isNetMessage;
  bool specialFlag;
  int iovIndex;
  int len;
  
  isNetMessage = (bool) ((0x20 & buff[0]) >> 5);
  *nextSize = ((buff[0]) & 0x1F);
  specialFlag = (bool) ((0x40 & buff[0]) >> 6);

  /*std::cout << "isNetMessage:" << isNetMessage << std::endl;
  std::cout << "nextSize:" << *nextSize << std::endl;
  std::cout << "specialFlag:" << specialFlag << std::endl;*/
  
  if (isNetMessage) {
    if (specialFlag) {
      //Primitive type
      netMessage = new NetMessage();
      netMessage->primitiveSize = primitiveSizeArray[*nextSize];
      netMessage->type = *nextSize;
      *nextSize = netMessage->primitiveSize;
      netMessage->primitiveValue = (char*) malloc(netMessage->primitiveSize);
    } else {
      // Normal NetMessage
      *nextSize += typeHeader;
      // If first buffer is empty -> header for current netmessage
      if (data[0].iov_len == 0) {
        netMessage = this;
      // otherwise, nested netmessage
      } else {
        netMessage = new NetMessage();
      }
    }
    
    iovIndex = 0;
    returnNetMessage = netMessage;

  // Simple Block of data
  } else {
    netMessage = this;
    // toFree is relevant for data block only
    if (toFree != NULL) {
      *toFree = specialFlag;
    }
    iovIndex = iovCount;
    addIovec(1);
  }

  len = flagsHeader + *nextSize;
  netMessage->data[iovIndex].iov_len = len;
  netMessage->data[iovIndex].iov_base = (char*) malloc(len);
  memcpy(netMessage->data[iovIndex].iov_base, buff, flagsHeader);
  netMessage->freeIov[iovIndex] = true;

  return returnNetMessage;
}

void NetMessage::parseTypeSize(const char *buff, bool withType, size_t size,
  size_t *blockSize) {
  int index = 0;
  swap_64 swap64;
  
  memcpy(&(data[iovCount-1].iov_base[flagsHeader]), buff, size);
  
  
  if (primitiveSize != 0) {
    memcpy(primitiveValue, buff, size);
    *blockSize = 0;
  } else {
    // Type
    if (withType) {
      type = convertToUInt16(buff);
      index += 2;
      size -= 2;
    }
    //std::cout << "type:" << type << std::endl;
    
    // Size
    swap64.uint = 0;
    if (isLittleEndian()) {
      //std::cout << "Receive bytes: ";
      for (size_t i = size; i > 0; --i) {
        swap64.chars[size-i] = buff[index + i -1];
        //std::cout << buff[index + i - 1];
      }
      //std::cout << std::endl;
    } else {
      memcpy(&(swap64.chars[8-size]), &(buff[index]), size);
    }
    *blockSize = swap64.uint;
    //std::cout << "blockSize:" << *blockSize << std::endl;
  }
}

void NetMessage::deleteAllData(void) {
  std::vector<NetMessage*>::iterator iter;

  for (int i = 0; i<iovCount; ++i) {
    free(data[i].iov_base);
  }
  iovCount = 0;

  for (iter = nestedNetMessages.begin(); iter != nestedNetMessages.end(); ++iter) {
    (*iter)->deleteAllData();
    delete *iter;
  }
  nestedNetMessages.clear();
}
