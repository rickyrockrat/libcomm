#include "file.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

const int MAX_IOV = sysconf(_SC_IOV_MAX);

void File::openFile(const char *path, int access, int flags, mode_t mode) {
  fd = open(path, access | flags, mode); 

  if (fd == -1) {
    throw FileException::Exception(errno);
  }
  this->path = std::string(path);
  constructAbsolutePath();
}

void File::constructAbsolutePath(void) {
  size_t sizeBuffer = 0;
  char *buffer = NULL;

  do {
    sizeBuffer += 96;
    buffer = (char*) realloc(buffer, sizeBuffer);
  } while (getcwd(buffer,sizeBuffer) == NULL);

  absolutePath = std::string(buffer);
  free(buffer);
}

File::File(std::string path) {
  openFile(path.c_str(), DEFAULT_ACCESS, DEFAULT_FLAGS, DEFAULT_MODE); 
}

File::File(std::string path, int access) {
  openFile(path.c_str(), access, DEFAULT_FLAGS, DEFAULT_MODE); 
}

File::File(std::string path, int access, int flags) {
  openFile(path.c_str(), access, flags, DEFAULT_MODE); 
}

File::File(std::string path, int access, int flags, mode_t mode) {
  openFile(path.c_str(), access, flags, mode); 
}

const struct stat &File::getStats(void) {
  if (fstat(fd, &stats) == -1) {
    throw File::FileException(errno);
  }
  return stats; 
}


const std::string &File::getAbsolutePath(void) {
  return absolutePath;
}

const std::string &File::getPath(void) {
  return path;
}

mode_t File::getUmask(void) {
  mode_t u = umask((mode_t) 0);
  umask(u);

  return u;
}

void File::setUmask(mode_t u) {
  umask(u);
}

bool File::exists(std::string path) {
  try {
    getStats(path);
    return true;
  } catch (Exception &e) {
    return false;
  }
}

struct stat File::getStats(std::string path) {
  struct stat stats;
  if (stat(path.c_str(), &stats) == -1) {
    throw File::FileException(errno);
  }
  return stats;
}

File::FileException::FileException(int code): Exception(code) {
}

File::FileException::FileException(int code, std::string message): Exception(code, message) {
}

RandomAccessFile::RandomAccessFile(std::string path): File(path) {
}

RandomAccessFile::RandomAccessFile(std::string path, int access): File(path, access) {
}

RandomAccessFile::RandomAccessFile(std::string path, int access, int flags)
  : File(path, access, flags) {
}

RandomAccessFile::RandomAccessFile(std::string path, int access, int flags, mode_t mode)
  : File(path, access, flags, mode) {
}

ssize_t RandomAccessFile::writeData(  const char *data, size_t size, int flags,
                                      const NetAddress *addr){
  ssize_t bytesWritten;
  bytesWritten = write(fd, data, size);

  if (bytesWritten == -1) {
    throw OutputStream::OutputStreamException(errno, size);
  }

  return bytesWritten;
}

ssize_t RandomAccessFile::writeData(  const struct iovec *iov, int iovcnt,
                                      const NetAddress *addr) {
  int currentIovCnt;
  int totalIovCnt = 0;
  size_t totalQuantityWritten = 0;
  ssize_t quantityWritten = 0;

  while (totalIovCnt < iovcnt) {
    currentIovCnt = iovcnt - totalIovCnt;
    if (currentIovCnt > MAX_IOV) currentIovCnt = MAX_IOV;

    quantityWritten = writev(fd, &(iov[totalIovCnt]), currentIovCnt);
    if (quantityWritten == -1) {
      size_t toWrite;
      char *dataLeft;
      
      dataLeft = generateRemainingData(iov,iovcnt, totalQuantityWritten, &toWrite);
      throw OutputStream::OutputStreamException(errno, toWrite,  dataLeft, quantityWritten);
    }
    totalQuantityWritten += quantityWritten;
    totalIovCnt += currentIovCnt;
  }
  
  return totalQuantityWritten;
}



ssize_t RandomAccessFile::readData(   char *buffer, size_t size, int flags,
                                      NetAddress *addr) {
  ssize_t bytesRead;
  bytesRead = read(fd, buffer, size);
 
  switch (bytesRead) {
    case -1:
      throw InputStream::InputStreamException(errno);
    case 0:
      throw InputStream::InputStreamException(EX_EOF, "End of file.");
    default:
      return bytesRead;
  }
}

ssize_t RandomAccessFile::peekData(   char *buffer, size_t size,
                          NetAddress *addr) {
  ssize_t bytesRead;

  bytesRead = readData(buffer, size, 0, NULL);
  seekOffset(-bytesRead);

  return bytesRead;
}



void RandomAccessFile::seekOffset(off_t offset, int from) {
  off_t result;
  
  if ((offset < 0) && ((from == current) || (from == end))) {
    off_t currentOffset = lseek(fd, 0, from);
    offset = currentOffset + offset;
    from = start;
  }

  result = lseek(fd, offset, from);
  if (result == -1) {
    throw File::FileException(errno);
  }
}

BufferedFile::BufferedFile(std::string path): File(path) {
}

BufferedFile::BufferedFile(std::string path, int access): File(path, access) {
}

BufferedFile::BufferedFile(std::string path, int access, int flags)
  : File(path, access, flags) {
}

BufferedFile::BufferedFile(std::string path, int access, int flags, mode_t mode)
  : File(path, access, flags, mode) {
}

ssize_t BufferedFile::writeRawData(  const struct iovec *iov, int iovcnt,
                                      const NetAddress *addr) {
  int currentIovCnt;
  int totalIovCnt = 0;
  size_t totalQuantityWritten = 0;
  ssize_t quantityWritten = 0;

  while (totalIovCnt < iovcnt) {
    currentIovCnt = iovcnt - totalIovCnt;
    if (currentIovCnt > MAX_IOV) currentIovCnt = MAX_IOV;

    quantityWritten = writev(fd, &(iov[totalIovCnt]), currentIovCnt);
    if (quantityWritten == -1) {
      size_t toWrite;
      char *dataLeft;
      
      dataLeft = generateRemainingData(iov,iovcnt, totalQuantityWritten, &toWrite);
      throw OutputStream::OutputStreamException(errno, toWrite,  dataLeft, quantityWritten);
    }
    totalQuantityWritten += quantityWritten;
    totalIovCnt += currentIovCnt;
  }
  
  return totalQuantityWritten;
}

ssize_t BufferedFile::readRawData(   char *buffer, size_t size, int flags,
                          NetAddress *addr) {
  ssize_t bytesRead;
  bytesRead = read(fd, buffer, size);
 
  switch (bytesRead) {
    case -1:
      throw InputStream::InputStreamException(errno);
    case 0:
      throw InputStream::InputStreamException(EX_EOF, "End of file.");
    default:
      return bytesRead;
  }
}
