#ifndef TEST_AUTO_SER_H
#define TEST_AUTO_SER_H

#include "libcomm/libcomm_structs.h"

#define className TestAutoSerializable
class TestAutoSerializable : public AutoSerializable {
  public:
    TestAutoSerializable( uint64_t a, double *b, String str, 
                          String *strPtr, Vector<char> *v);
    ~TestAutoSerializable();
    TestAutoSerializable();
    bool operator==(TestAutoSerializable &tas);
  protected :
    virtual void copyFields(const TestAutoSerializable &t);
  //Serialization stuff
  AUTO_SER_PRIMITIVE_FIELD_WITH_GET_AND_SET(uint64_t,a)
  AUTO_SER_PRIMITIVE_FIELD_PTR_WITH_GET_AND_SET(double,b)
  AUTO_SER_FIELD_WITH_GET_AND_SET(String,str)
  AUTO_SER_FIELD_PTR_WITH_GET_AND_SET(String,strPtr)
  AUTO_SER_FIELD_PTR_WITH_GET_AND_SET(Vector<char>,v) 

  AUTO_SER_STUFF(ITEM(a),ITEM(b),ITEM(str),ITEM(strPtr),ITEM(v)) 
};
#undef className

#endif
