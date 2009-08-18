//! \file simple_serializable.h
//! \brief Automatic simple Serializable class
//! \author Alexandre Roux
//! \version 0.1
//! \date 13 mars 2009
//!
//! File containing the declarations of the class SimpleSerializable and some
//! code generation macro.

#ifndef SIMPLE_SERIALIZABLE_H
#define SIMPLE_SERIALIZABLE_H

#include "../serializable.h"

#define SIMPLE_SER_STUFF \
  int returnClassSize() const { \
    const className &ref = *this; \
    return sizeof(ref)-sizeof(int); \
  } \
  static uint16_t type; \
  virtual uint16_t getType() const { \
    return type; \
  } \
  static Serializable *deserialize(const NetMessage &data, bool ptr) { \
  return (SimpleSerializable::deserialize( \
      new className(),data,ptr)); \
  } \
  friend class libcomm;

#define SIMPLE_SER_INIT \
  memset((void*)&(((char*)this)[sizeof(int)]),0,(size_t)returnClassSize());

#define SIMPLE_SER_STATIC_STUFF \
  uint16_t className::type = 0;
  

//! \class SimpleSerializable libcomm/structs/simple_serializable.h
//! \brief Automatic simple Serializable class
//!
//! This class provides serialization methods which automatically
//! serialize all fields of the object. However, it is only a "shallow"
//! serialization: only fields bytes are copied. There is not distinction
//! between real data and pointers.
//!
//! If you want automatic "deep" serialization, use AutoSerializable instead.
//!
//! To have a valid SimpleSerializable subclass, you have to :
//! \li include the simple_serializable.h header and make your class inherit from
//!     SimpleSerializable;
//! \li define the macro className as the name of your class (undef it at class end);
//! \li declare and define the default constructor with no argument;
//! \li call the SIMPLE_SER_STUFF;
//! \li call the AUTO_SIMPLE_STATIC_STUFF macro outside of the class;
//!
//! That's it.
//!
//! Here is an example of SimpleSerializable subclass:
//! \include examples/simple_serializable_example.h

class SimpleSerializable : public Serializable {                                
  private :
    static uint16_t type;
    
    virtual int returnClassSize(void) const = 0;
    NetMessage *serialize() const;
    virtual uint16_t getType() const = 0;
    static Serializable *deserialize(const NetMessage &data, bool ptr);        

    friend class SerializationManager;
    friend class libcomm;

  protected :
    SimpleSerializable();
    static Serializable *deserialize(SimpleSerializable *ss, 
      const NetMessage &data, bool ptr);

  public :
    virtual ~SimpleSerializable();
 
};

#endif
