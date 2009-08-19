#include <libcomm/libcomm.h>
#include <libcomm/libcomm_structs.h>
#include <libcomm/structs/auto_serializable.h>
#include <libcomm/structs/string_serializable.h>
#include <libcomm/structs/buffer_serializable.h>
#include <libcomm/tcp_socket.h>
#include <libcomm/file.h>
#include <libcomm/exception.h>

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include <sys/time.h>

#include <sstream>

#define USER_PER_SEC 1000000
#define NBR_OF_TESTS 100

#define size_buffer 4096

void print_usage(void) {
  std::cout << "usage: myftp [--file file] | [--host host] [--port port]" << std::endl;
}

Buffer<char> *read_next(BufferedFile &file) {
  try {
    return file.readBytes(NULL);
  } catch (Exception &e) {
    if (e.getCode() == EX_EOF) {
      return NULL;
    }
  }
}

uint64_t get_time(void) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  uint64_t start_time = tv.tv_sec * USER_PER_SEC + tv.tv_usec;

  return start_time;
}

int main(int argc, char **argv) {
  bool server = true;
  std::string server_address;
  int port = 5555;
  std::string file_name = "default.txt";
  uint64_t t;
  uint64_t first_t;
  uint64_t total_t;
  std::stringstream ss;
  int i = 1;
  
  if (argc == 2) { 
    if (!strcmp(argv[1], "--help")) {
      print_usage();
      exit(EXIT_SUCCESS);
    } else {
      std::cout << "Unknown option \"" << argv[1] << "\"" << std::endl;
      print_usage();
      exit(EXIT_FAILURE);
    }
  }

  while ((i+1) < argc) {
    std::string opt(argv[i]);
    ++i;
    if (opt == "--host") {
      server = false;
      server_address = std::string(argv[i]);
    } else if (opt == "--file") {
      file_name = std::string(argv[i]);
    } else if (opt == "--port") {
      ss << argv[i];
      ss >> port;
      ss.clear();
    } else {
      std::cout << "Unknown option \"" << opt << "\"" << std::endl;
      print_usage();
      exit(EXIT_FAILURE);
    }
    ++i;
  }

  libcomm::init();

  TcpSocket *socket;
  BufferedFile *file;

  if (!server) {
    uint64_t start_time;
    uint64_t end_time;
    size_t downloaded_size = 0;
    double througput;

    NetAddress addr(server_address, port);
    try {
  
      socket = new TcpSocket(addr);
      
      start_time = get_time();
      String *file_name_ptr = socket->readString();
      file_name = *file_name_ptr;
      std::cout << *file_name_ptr << std::endl;
      delete file_name_ptr;

      file = new BufferedFile(file_name, File::w, File::create);

      for (;;) {
        Serializable *ser = socket->readObject();
        Buffer<char> *buff = dynamic_cast<Buffer<char>*>(ser);
        if (buff) {
          file->writeBytes(*buff);
          downloaded_size += buff->size();
          delete buff;
        } else {
          std::cerr << "Received wrong data from server." << std::endl;
          exit(EXIT_FAILURE);
        }
      }
    } catch (Exception &e) {
      if (e.getCode() != EX_STREAM_CLOSED) {
        std::cerr << e.getCode() << ": " << e.getMessage() << std::endl; 
        exit(EXIT_FAILURE);
      }
      file->flushBuffers();
    }

    end_time = get_time();
    througput = (double) downloaded_size / (double) (end_time - start_time);

    std::cout << "File \"" << file_name << "\" (" << downloaded_size 
      << " bytes) has been successfully downloaded in " << (end_time - start_time) 
      << " µsecs.\nThrougput: " << througput << " bytes/µsec (" 
      << througput * USER_PER_SEC / (double) (1024 * 1024) << " mbytes/sec)" << std::endl;
    
  } else {
    Buffer<char> *buf;

    file = new BufferedFile(file_name, File::r);
    
    try {
      TcpServerSocket ssocket(port);

      socket = ssocket.acceptConnection();

      socket->writeString(file_name);
      while ((buf = read_next(*file)) != NULL) {
        socket->writeObject(*buf);
        delete buf;
      }

      ssocket.closeServer();
    } catch (Exception &e) {
      std::cerr << e.getCode() << ": " << e.getMessage() << std::endl; 
      exit(EXIT_FAILURE);
    }

  }
  socket->closeStream();
  delete socket;
  file->closeStream();
  delete file;

  libcomm::clean();

  return EXIT_SUCCESS;
}
