#include <sys/socket.h>  // socket()
#include <netinet/in.h>  // sockaddr_in
#include <netdb.h>       // gethostbyname(), htons()
#include <iostream>

int main (char** args) {
  std::string str = "1234567890abcdefghijklmonpqrstuvwxyz";
  int socketId = socket ( AF_INET , SOCK_DGRAM, 0 ) ;
  sockaddr_in serverAddr, clientAddr;
  const sockaddr *serverAddCast = (sockaddr *) &serverAddr;
  const sockaddr *clientAddrCast = (sockaddr *) &clientAddr;

  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(4444);

  struct hostent *hp = gethostbyname("localhost");
  memcpy((void*)&serverAddr.sin_addr,(void*) hp->h_addr, hp->h_length);
  
  int size = sizeof(serverAddr);
  sendto(socketId, str.c_str(), str.length(), 0, serverAddCast, size);

  close(socketId);
  return 0;
}
