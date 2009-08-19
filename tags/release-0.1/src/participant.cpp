#include "participant.h"

Participant::Participant() {}

Participant::Participant(uint16_t id, std::string &address) {
  this->id = id;
  this->address = address;
}

uint16_t Participant::getId() const {
  return id;
}

const std::string &Participant::getAddress() const {
  return address;
}
