#ifndef PARTICIPANT_H
#define PARTICIPANT_H

#include <string>
#include <stdint.h>

class Participant {
  private :
    uint16_t id;
    std::string address;

  public :
    Participant(uint16_t id, std::string &address);
    Participant();
    uint16_t getId() const;
    const std::string &getAddress() const;
};

#endif
