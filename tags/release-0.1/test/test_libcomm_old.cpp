#include <iostream>
#include <sstream>
#include <vector>

/*"
#include "udp_message.h"
#include "serialization_manager.h"
#include "udp_socket.h"
#include "time.h"*/

#include "libcomm.h"
#include "structs/string.h"
#include "structs/vector.h"
#include "thread.h"

void test_serialization() {
  /*String sm("Petit test ahaha");
  std::cout << sm.getMessage() << std::endl;
  SerializationManager &serManager = SerializationManager::getSerializationManager();
  UdpMessage *um = serManager.serialize(sm);
  std::cout << um->getType() << std::endl;
  std::cout << um->getTimeStamp() << std::endl;
  std::cout << um->getData() << ":" << um->getData().length() << std::endl;
  std::string str = um->getRawRepresentation();
  std::cout << str << ":" << str.length() << std::endl;
  UdpMessage um2(str.c_str(), str.length());
  std::cout << um2.getType() << std::endl;
  std::cout << um2.getTimeStamp() << std::endl;
  std::cout << um2.getData() << ":" << um->getData().length() << std::endl;
  Serializable *ser = serManager.deserialize(um2);
  String *sm2 = (String*) ser;
  std::cout << sm2->getMessage() << std::endl;*/
}

class ServerThread : public Thread {
 
  private :

  public :
    virtual ~ServerThread();
    void *run();
};


int main(int argc, char **argv) {
  
  libcomm::init();
  SerializationManager *sm = SerializationManager::getSerializationManager();

  /*if (argc == 2) {
    int port;
    std::stringstream s2(argv[1]);
    s2 >> port;

    try {

      UdpSocket socket(port);
    
      std::vector<Thread*> threads = std::vector<Thread*>();
      int i = 0;
      while ( i < 5) {
        UdpAddress *addr = new UdpAddress();
        Serializable *object = socket.receiveObject(*addr);
        Vector<String> *mess = (Vector<String>*) object;
        ServerThread *st = new ServerThread(mess, addr);
        threads.push_back(st);
        st->start();
        ++i;
      }

      std::vector<Thread*>::iterator iter = threads.begin();

      for (; iter != threads.end(); ++iter) {
        Thread *thread = *iter;
        thread->join();
        delete thread;
      }
      
      socket.closeSocket();
    
    } catch (UdpSocket::UdpException &e) {
      e.printCodeAndMessage();
    }

  } else if (argc == 4) {
    std::string address(argv[1]);

    int port;
    std::stringstream s2(argv[2]);
    s2 >> port;

    int port2;
    std::stringstream s3(argv[3]);
    s3 >> port2;

    try {
      
      UdpSocket socket(port2);
      
      Vector<String> v; 
      String sm1("Test send message1");
      String sm2("Test send message2");
      String sm3("Test send message3");
      v.push_back(sm1);
      v.push_back(sm2);
      v.push_back(sm3);
      UdpAddress addr(address,port);
      socket.sendObject(v, addr);
      
      socket.closeSocket();
    } catch (UdpSocket::UdpException &e) {
      e.printCodeAndMessage();
    }
  } else if (argc == 1) {
    
    Vector<String> v; 
    String mess1("Premier message");
    String mess2("Deuxième message");
    String mess3("Troisième message");
    String mess4("Troisième message");
    String mess5("Troisième message");
    String mess6("Troisième message");
    String mess7("Troisième message");
    String mess8("Troisième message");
    String mess9("Troisième message");
    
    v.push_back(mess1);
    v.push_back(mess2);
    v.push_back(mess3);
    v.push_back(mess4);
    v.push_back(mess5);
    v.push_back(mess6);
    v.push_back(mess7);
    v.push_back(mess8);
    v.push_back(mess9);

    SerializationManager *serManager = SerializationManager::getSerializationManager();
    UdpMessage *message = serManager->serialize(v);
    std::cout << message->getRawRepresentation() << std::endl;
    Vector<String> *v2 = (Vector<String>*)serManager->deserialize(*message);
    
    for (size_t i = 0; i<v2->size(); ++i) {
      std::cout << (*v2)[i] << std::endl;
    }
    
  }*/

  libcomm::clean();

  return 0;
}


ServerThread::ServerThread(Vector<String> *messages, UdpAddress *address) {
  this->messages = messages;
  this->address = address;
}

void *ServerThread::run() {
  std::cout << "Message received from " << address->getAddress() << ":"
            << address->getPort() << "  :  " << messages->size() << std::endl;
            
    for (size_t i = 0; i<messages->size(); ++i) {
      std::cout << (*messages)[i] << std::endl;
    }
  return NULL;
}

ServerThread::~ServerThread(){
  delete messages;
  delete address;
}
