//simple_serializable_example.h
#ifndef DUMMY_CLASS_H
#define DUMMY_CLASS_H

#include <libcomm/structs/simple_serializable.h>

#define className DummmyClass
class DummmyClass : public SimpleSerializable {
  public :
    uint16_t a;
    uint32_t b;
    uint64_t c;
    char d;
    float e;
    double f;
  private :
    DummmyClass() {}
    SIMPLE_SER_STUFF
};
SIMPLE_SER_STATIC_STUFF
#undef className

#endif
