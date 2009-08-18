#ifndef FILE_H
#define FILE_H

#include "input_stream.h"
#include "output_stream.h"

#include <fcntl.h>

#define DEFAULT_ACCESS (r)
#define DEFAULT_FLAGS (0)
#define DEFAULT_MODE (0666)


class File : public OutputStream {

  private:
    struct stat stats;
    std::string path;
    std::string absolutePath;

    ssize_t writeData(  const char *data, size_t size, int flags,
                        const NetAddress *addr);
    ssize_t writeData(  const struct iovec *iov, int iovcnt,
                        const NetAddress *addr);

    void openFile(const char *path, int access, int flags, mode_t mode);
    void constructAbsolutePath(void);

  protected:
    class FileException: public Exception {
      public:
        FileException(int code);
        FileException(int code, std::string message);
    };

  public:
    enum access{
      r,
      w,
      rw
    };

    enum flags {
      create = O_CREAT,
      append = O_APPEND,
      large = O_LARGEFILE,
      noatime = O_NOATIME,
      noCache = O_DIRECT,
      sync = O_SYNC,
      trunc = O_TRUNC
    };

    enum mode {
      oX     = 0001,
      oW     = 0002,
      oWX    = 0003,
      oR     = 0004,
      oRX    = 0005,
      oRW    = 0006,
      oRWX   = 0007,
      gX     = 0010,
      gW     = 0020,
      gWX    = 0030,
      gR     = 0040,
      gRX    = 0050,
      gRW    = 0060,
      gRWX   = 0070,
      uX     = 0100,
      uW     = 0200,
      uWX    = 0300,
      uR     = 0400,
      uRX    = 0500,
      uRW    = 0600,
      uRWX   = 0700,
      aX     = 0111,
      aW     = 0222,
      aWX    = 0333,
      aR     = 0444,
      aRX    = 0555,
      aRW    = 0666,
      aRWX   = 0777
    };

    enum fromOffset {
      start = SEEK_SET,
      current = SEEK_CUR,
      end = SEEK_END
    };
    
    File(std::string path);
    File(std::string path, int access);
    File(std::string path, int access, int flags);
    File(std::string path, int access, int flags, mode_t mode);

    const struct stat &getStats(void);

    const std::string &getAbsolutePath(void);
    const std::string &getPath(void);

    static mode_t getUmask(void);
    static void setUmask(mode_t u);
    
    static bool exists(std::string path);
    static struct stat getStats(std::string path);
};

class RandomFile: public File, public InputStream {
  private:
    ssize_t readData(   char *buffer, size_t size, int flags,
                        NetAddress *addr);
    ssize_t peekData(   char *buffer, size_t size,
                        NetAddress *addr);
  public:
    void seekOffset(off_t offset, int from = current);
};

class BufferedFile : public File, public BufferedInputStream {
  private:
    ssize_t readRawData(  char *buffer, size_t size, int flags,
                          NetAddress *addr);
};

#endif
