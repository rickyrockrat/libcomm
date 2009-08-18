#ifndef UDP_MESSAGE_H
#define UDP_MESSAGE_H

#include <string>

#define UDP_PACKET_MAX_SIZE 1500
#define ETHER_IP_HEADERS 42 //ahem!

class UdpMessage {
  
  private :
    time_t timeStamp;
    uint16_t type;
    uint16_t dataSize;
    char *data;

  public :
    UdpMessage(uint16_t type, char *data, uint16_t dataSize);
    UdpMessage(const char *buffer, size_t bufferAvailable, size_t *byteRead, bool fetchTs);
    ~UdpMessage();
    uint16_t getType() const;
    void setType(uint16_t type);
    uint16_t getDataSize() const;
    time_t getTimeStamp() const;
    const char *getData() const;
    void getRawRepresentation(bool withTs, char *c) const;
    char *getRawRepresentation(bool withTs, size_t &size) const;
    void addData(uint16_t type, const char *newData, size_t size);
    void addData(const UdpMessage &message);
    void addData(const char *newData, size_t size);

};

#endif
