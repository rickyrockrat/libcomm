#include <sys/types.h>   // htons()
#include <sys/socket.h>  // socket()
#include <netinet/in.h>  // sockaddr_in
#include <netdb.h>       // gethostbyname()
#include <iostream>

#define bufferSize 40

int main (char** args) {
  char* buff = new char[bufferSize * sizeof(char)];
  int socketId = socket ( AF_INET , SOCK_DGRAM, 0 ) ;
  sockaddr_in serverAddr, clientAddr;
  const sockaddr *serverAddCast = (sockaddr *) &serverAddr;
  sockaddr *clientAddrCast = (sockaddr *) &clientAddr;

  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(4444);
  serverAddr.sin_addr.s_addr = INADDR_ANY;

  socklen_t size = sizeof(clientAddr);
  bind (socketId, serverAddCast, sizeof(serverAddr));
  recvfrom (socketId, buff, bufferSize, 0, clientAddrCast, &size);

  std::cout << std::string(buff) << std::endl; 

  close(socketId);
  return 0;
}
