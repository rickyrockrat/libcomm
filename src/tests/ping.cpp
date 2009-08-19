#include <iostream>
#include <stdlib.h>
#include <libcomm/libcomm.h>
#include <libcomm/udp_socket.h>
#include <libcomm/tcp_socket.h>
#include <libcomm/timer.h>
#include <libcomm/structs/simple_serializable.h>

static bool go = true;

#define className PingRequest 
class PingRequest : public SimpleSerializable {
  public :
    PingRequest(uint32_t number, uint64_t timer) {
      SIMPLE_SER_INIT;
      this->number = number;
      this->timer = timer;
    }
    uint64_t timer;
    uint32_t number;
  private :
    PingRequest() {}
    SIMPLE_SER_STUFF
};
SIMPLE_SER_STATIC_STUFF
#undef className

void printUsageAndExit() {
  std::cout << "Usage: ping protocol\n"
            << "       ping protocol host"
            << std::endl;
  exit(-1);
}

int main(int argc, char ** argv) {
  
  if ((argc != 2) && (argc != 3)) {
    printUsageAndExit();
  }

  libcomm::init();
  libcomm::addSupportFor(MyType<PingRequest>());

  if (argc == 2) {
    //Server
    std::string protocol(argv[1]);
    if (protocol == "tcp") {
      //TCP server
      TcpServerSocket ssocket(5555);
      while (go) {
        try {
          TcpSocket *socket = ssocket.acceptConnection(500000000);
          NetAddress addr = socket->getDistantAddress();
          std::cout << "Incomming connection from " << addr.getAddress() << ":" 
            << addr.getPort() << std::endl;
          bool go_inside = true;
          while (go_inside) {
            try {
              Serializable *ser = socket->readObject(500000000); 
              PingRequest *pr = dynamic_cast<PingRequest*>(ser);
              if (pr) {
                socket->writeObject(*pr);
                std::cout << "Received ping request with number " << pr->number
                  << std::endl;
                delete pr;
              } else {
                delete ser;
              }
            } catch (Stream::StreamException &e) {
              switch (e.getCode()) {
                case EX_STREAM_CLOSED:
                  std::cout << "Connection with " << addr.getAddress() << ":" 
                    << addr.getPort() << " closed" << std::endl;
                  go_inside = false;
                  break;
                case EX_STREAM_TIMEOUT:
                  break;
              }
            }
          }
          socket->closeStream();
        } catch (Exception &e) {
          switch (e.getCode()) {
            case EX_STREAM_TIMEOUT:
              break;
          }
        }
      }
    ssocket.closeServer();
    //Udp server
    } else {
      UdpSocket socket(5555);
      NetAddress addr;
      while (go) {
        Serializable *ser = socket.readObject(&addr); 
        if (ser != (Serializable*) NULL) {
          PingRequest *pr = dynamic_cast<PingRequest*>(ser);
          if (pr) {
            socket.writeObject(*pr,addr);
            std::cout << "Received ping request with number " << pr->number
              << " from " << addr.getAddress() << ":" << addr.getPort()
              << std::endl;
            delete pr;
          } else {
            delete ser;
          }
        }
      }
      socket.closeStream();
    }
  } else if (argc == 3) {

    Timer timer;
    timer.startTimer();
    std::string protocol(argv[1]);
    
    if (protocol == "tcp") {
      NetAddress add(argv[2],5555);
      try {
        TcpSocket socket;
        socket.connectSocket(add,1,0);
        uint32_t seqNbr = 1;
        
        while (go) {
          PingRequest pr(seqNbr,timer.getTime());
          socket.writeObject(pr);
          Serializable *ser = socket.readObject(); 
          uint64_t timerValue = timer.getTime();
          if (ser != (Serializable*) NULL) {
            PingRequest *pr = dynamic_cast<PingRequest*>(ser);
            if (pr) {
              uint64_t t = pr->timer -timerValue;
              std::cout << "Send and receive back ping resquest : " 
                << t/1000000.0 << std::endl;
              if (t < 1000000) {
                usleep(1000000-t);
              } else {
                usleep(1000000);
              }
              delete pr;
            } else {
              delete ser;
            }
          }
          ++seqNbr;
        }
        socket.closeStream();
      } catch (Exception &e) {
        e.printCodeAndMessage();
      }
    } else {
      UdpSocket socket(4444);
      NetAddress add(argv[2],5555);
      uint32_t seqNbr = 1;
      
      while (go) {
        PingRequest pr(seqNbr,timer.getTime());
        socket.writeObject(pr,add);
        Serializable *ser = socket.readObject(NULL); 
        uint64_t timerValue = timer.getTime();
        if (ser != (Serializable*) NULL) {
          PingRequest *pr = dynamic_cast<PingRequest*>(ser);
          if (pr) {
            uint64_t t = pr->timer -timerValue;
            std::cout << "Send and receive back ping resquest : " 
              << t/1000000.0 << std::endl;
            if (t < 1000000) {
              usleep(1000000-t);
            } else {
              usleep(1000000);
            }
            delete pr;
          } else {
            delete ser;
          }
        }
        ++seqNbr;
      }
      socket.closeStream();
    }
  }
  libcomm::clean();

}
