//! \file auto_serializable.h
//! \brief Automatic Serializable class
//! \author Alexandre Roux
//! \version 0.1
//! \date 13 mars 2009
//!
//! File containing the declarations of the class AutoSerializable and some
//! code generation macro.

#ifndef AUTO_SERIALIZABLE_H
#define AUTO_SERIALIZABLE_H

#include "../libcomm.h"
#include "../types_utils.h"
#include <stdarg.h>
#include <vector>
#include <iostream>

#define _QUOTEME(x) #x
#define QUOTEME(x) _QUOTEME(x)

//! \class AutoSerializable libcomm/structs/auto_serializable.h
//! \brief Automatic Serializable class
//!
//! This class, used with some macros (defined in auto_serializable.h), provides
//! an easy way to define custom serializable classes.
//!
//! To have a valid AutoSerializable subclass, you have to :
//! \li include the auto_serializable.h header and make your class inherit from
//!     AutoSerializable;
//! \li define the macro className as the name of your class (undef it at class end);
//! \li declare and define the default constructor with no argument;
//! \li declare the fields of your class using the AUTO_SER marcos;
//! \li call the AUTO_SER_STUFF macro with the fields you want to be serialized;
//! \li call the AUTO_SER_STATIC_STUFF macro outside of the class;
//!
//! Here is an example of a simple user defined serializable class representing 
//! a client request:
//! \include examples/auto_serializable_example.h
//! \include examples/auto_serializable_example.cpp
//!
//! There are several AUTO_SER macros to declare fields:
//! \li AUTO_SER_FIELD(type,name): primitive or object member;
//! \li AUTO_SER_FIELD_PTR: pointer to a primitive or an object value;
//! \li AUTO_SER_PRIMITIVE_FIELD: primitive only member;
//! \li AUTO_SER_PRIMITIVE_FIELD_PTR: pointer to a primitive only value.
//!
//! Note that macros declaring pointer fields take the value type as first
//! argument, and not the pointer one. In the example, we declare a field
//! of type Serializable*, but the type passed in the macro is Serializable.
//!
//! Even if AUTO_SER_FIELD and AUTO_SER_FIELD_PTR can be used with primitive
//! fields (char, uint8_t, ...), it is better to use AUTO_SER_PRIMITIVE_FIELD
//! and AUTO_SER_PRIMITIVE_FIELD_PTR because they are more space saving. The 
//! first macros are generic: they serialize fields with headers and content.
//! The second macros are specialized for primitive fields: they use a simple
//! mecanism to serialize fields which doesn't need the use of headers.
//!
//! Each of these macros have four variants: one which adds a getter, one which
//! adds a setter, one which adds both and the last one which can be used with types
//! containing commas (for instance, Map<char,char>). If your type has more than
//! one comma, you can use typedef to avoid macros errors.
//!
//! The AUTO_SER_STUFF() macro generates the code which will serialize each
//! field one after the other. Field which are not given as argument will
//! not be serialized.  If your class has no fields, it is possible to use
//! the macro AUTO_SER_STUFF_WITHOUT_MEMBERS().
//!
//! AutoSerializable supports inheritance. If you want such an AutoSerializable
//! class hierarchy, each subclass will have to use the 
//! AUTO_SER_STUFF_WITH_SUPERCLASS() or
//! AUTO_SER_STUFF_WITH_SUPERCLASS_WITHOUT_MEMBERS() macros instead of the
//! AUTO_SER_STUFF() or, respectively, AUTO_SER_STUFF_WITHOUT_MEMBERS() macros.
//!
//! Here is an example:
//! \include examples/auto_sub_serializable_example.h
//! \include examples/auto_sub_serializable_example.cpp
//!
//! Note that multiple inheritance is not supported; AutoSerializable must be
//! the base class of only one superclass.
//!
//! Each user defined Serializable class must be register at program
//! initialization. AutoSerializable classes need to be registered with the
//! addSupportForAutoSerializable function.
//! \code
//!libcomm::addSupportForAutoSerializable(MyType<ClientRequest>());
//! \endcode
//! Of course, each nested Serializable object needs to be registered once too.
class AutoSerializable : public Serializable {
  private :
    static uint16_t type;
    
    virtual NetMessage *serialize() const = 0;
    virtual uint16_t getType() const = 0;
    static Serializable *deserialize(const NetMessage &data, bool ptr);         
    friend class SerializationManager;
    friend class libcomm;
  protected :
    //helpers
    char *cvrtochars(uint8_t uint, size_t &size);
    void cvrtochars(uint8_t uint, char *data, size_t &size);
    char *cvrtochars(uint16_t uint, size_t &size);
    void cvrtochars(uint16_t uint, char *data, size_t &size);
    char *cvrtochars(uint32_t uint, size_t &size);
    void cvrtochars(uint32_t uint, char *data, size_t &size);
    char *cvrtochars(uint64_t uint, size_t &size);
    void cvrtochars(uint64_t uint, char *data, size_t &size);
    char *cvrtochars(int8_t i, size_t &size);
    void cvrtochars(int8_t i, char *data, size_t &size);
    char *cvrtochars(int16_t i, size_t &size);
    void cvrtochars(int16_t i, char *data, size_t &size);
    char *cvrtochars(int32_t i, size_t &size);
    void cvrtochars(int32_t i, char *data, size_t &size);
    char *cvrtochars(int64_t i, size_t &size);
    void cvrtochars(int64_t i, char *data, size_t &size);
    char *cvrtochars(float f, size_t &size);
    void cvrtochars(float f, char *data, size_t &size);
    char *cvrtochars(double d, size_t &size);
    void cvrtochars(double d, char *data, size_t &size);

  protected :
    AutoSerializable();

  public :
    virtual ~AutoSerializable();
 
};

//MACROS

//! \def AUTO_SER_FIELD_WITH_GET(type,name)
//! \brief Add a serialisable field with a getter
//! \param type the type of the field
//! \param the name of the field
#define AUTO_SER_FIELD_WITH_GET(type,name) \
  AUTO_SER_FIELD(type,name) \
  public:\
  const type &get##name() const { \
    return name;\
  }

//! \def AUTO_SER_FIELD_WITH_GET_2(type1,type2,name)
//! \brief Add a serialisable field with a getter.
//! \param type the first part of the type
//! \param type the second part of the type
//! \param the name of the field
//!
//! This macro should be used for types with a comma, like Map<char,char>.
#define AUTO_SER_FIELD_WITH_GET_2(type1,type2,name) \
  AUTO_SER_FIELD_2(type1,type2,name) \
  public:\
  const type1,type2 &get##name() const { \
    return name;\
  }

//! \def AUTO_SER_FIELD_WITH_SET(type,name)
//! \brief Add a serialisable field with a setter
//! \param type the type of the field
//! \param the name of the field
#define AUTO_SER_FIELD_WITH_SET(type,name) \
  AUTO_SER_FIELD(type,name) \
  public:\
  void set##name(type name) { \
    this->name = name;\
  }

//! \def AUTO_SER_FIELD_WITH_SET_2(type1,type2,name)
//! \brief Add a serialisable field with a setter.
//! \param type the first part of the type
//! \param type the second part of the type
//! \param the name of the field
//!
//! This macro should be used for types with a comma, like Map<char,char>.
#define AUTO_SER_FIELD_WITH_SET_2(type1,type2,name) \
  AUTO_SER_FIELD_2(type1,type2,name) \
  public:\
  void set##name(type1,type2, name) { \
    this->name = name;\
  }


//! \def AUTO_SER_FIELD_WITH_GET_AND_SET(type,name)
//! \brief Add a serialisable field with a getter and a setter
//! \param type the type of the field
//! \param the name of the field
#define AUTO_SER_FIELD_WITH_GET_AND_SET(type,name) \
  AUTO_SER_FIELD(type,name) \
  public:\
  const type &get##name() const { \
    return name;\
  }\
  void set##name(type name) { \
    this->name = name;\
  }

//! \def AUTO_SER_FIELD_WITH_GET_AND_SET_2(type1,type2,name)
//! \brief Add a serialisable field with a getter and a setter.
//! \param type the first part of the type
//! \param type the second part of the type
//! \param the name of the field
//!
//! This macro should be used for types with a comma, like Map<char,char>.
#define AUTO_SER_FIELD_WITH_GET_AND_SET_2(type1,type2,name) \
  AUTO_SER_FIELD_2(type1,type2,name) \
  public:\
  const type1,type2 &get##name() const { \
    return name;\
  }\
  void set##name(type1,type2 name) { \
    this->name = name;\
  }

//! \def AUTO_SER_FIELD(type,name)
//! \brief Add a serialisable field
//! \param type the type of the field
//! \param the name of the field
#define AUTO_SER_FIELD(type,name) \
  private:\
  type name; \
  AUTO_SER_ENTRY_POINTS(name) \
  AUTO_SER_RETURN_SIZE_PRIMITIVE(name,type) \
  AUTO_SER_FIELD_SERIALIZE(name) \
  AUTO_SER_FIELD_DESERIALIZE(type,name) \

//! \def AUTO_SER_FIELD_2(type,name)
//! \brief Add a serialisable field
//! \param type the first part of the type
//! \param type the second part of the type
//! \param the name of the field
//!
//! This macro should be used for types with a comma, like Map<char,char>.
#define AUTO_SER_FIELD_2(type1,type2,name) \
  private:\
  type1,type2 name; \
  AUTO_SER_ENTRY_POINTS(name) \
  AUTO_SER_RETURN_SIZE_PRIMITIVE_2(name,type1,type2) \
  AUTO_SER_FIELD_SERIALIZE(name) \
  AUTO_SER_FIELD_DESERIALIZE_2(type1,type2,name) \


#define AUTO_SER_FIELD_PTR_WITH_GET(type,name) \
  AUTO_SER_FIELD_PTR(type,name)\
  public:\
  const type *get##name() const { \
    return name;\
  }

#define AUTO_SER_FIELD_PTR_WITH_GET_2(type1,type2,name) \
  AUTO_SER_FIELD_PTR(type1,type2,name)\
  public:\
  const type1,type2 *get##name() const { \
    return name;\
  }

#define AUTO_SER_FIELD_PTR_WITH_SET(type,name) \
  AUTO_SER_FIELD_PTR(type,name)\
  public:\
  void set##name(type *name) { \
    this->name = name;\
  }

#define AUTO_SER_FIELD_PTR_WITH_SET_2(type1,type2,name) \
  AUTO_SER_FIELD_PTR_2(type1,type2,name)\
  public:\
  void set##name(type1,type2 *name) { \
    this->name = name;\
  }

#define AUTO_SER_FIELD_PTR_WITH_GET_AND_SET(type,name) \
  AUTO_SER_FIELD_PTR(type,name)\
  public:\
  const type *get##name() const { \
    return name;\
  }\
  void set##name(type *name) { \
    this->name = name;\
  }

#define AUTO_SER_FIELD_PTR_WITH_GET_AND_SET_2(type1,type2,name) \
  AUTO_SER_FIELD_PTR_2(type1,type2,name)\
  public:\
  const type1,type2 *get##name() const { \
    return name;\
  }\
  void set##name(type1,type2 *name) { \
    this->name = name;\
  }

#define AUTO_SER_FIELD_PTR(type,name) \
  private:\
  type *name; \
  AUTO_SER_ENTRY_POINTS(name) \
  AUTO_SER_RETURN_SIZE_PRIMITIVE(name,type) \
  AUTO_SER_FIELD_SERIALIZE(name) \
  AUTO_SER_FIELD_DESERIALIZE_PTR(type,name) \

#define AUTO_SER_FIELD_PTR_2(type1,type2,name) \
  private:\
  type1,type2 *name; \
  AUTO_SER_ENTRY_POINTS(name) \
  AUTO_SER_RETURN_SIZE_PRIMITIVE_2(name,type1,type2) \
  AUTO_SER_FIELD_SERIALIZE(name) \
  AUTO_SER_FIELD_DESERIALIZE_PTR_2(type1,type2,name) \


#define AUTO_SER_PRIMITIVE_FIELD_WITH_GET(type,name) \
  AUTO_SER_PRIMITIVE_FIELD(type,name) \
  public:\
  const type &get##name() const { \
    return name;\
  }

#define AUTO_SER_PRIMITIVE_FIELD_WITH_SET(type,name) \
  AUTO_SER_PRIMITIVE_FIELD(type,name) \
  public:\
  void set##name(type name) { \
    this->name = name;\
  }

#define AUTO_SER_PRIMITIVE_FIELD_WITH_GET_AND_SET(type,name) \
  AUTO_SER_PRIMITIVE_FIELD(type,name) \
  public:\
  const type &get##name() const { \
    return name;\
  }\
  void set##name(type name) { \
    this->name = name;\
  }

#define AUTO_SER_PRIMITIVE_FIELD(type,name) \
  private:\
  type name; \
  AUTO_SER_ENTRY_POINTS(name) \
  AUTO_SER_RETURN_SIZE_PRIMITIVE(name,type) \
  AUTO_SER_PRIMITIVE_FIELD_SERIALIZE(name) \
  AUTO_SER_PRIMITIVE_FIELD_DESERIALIZE(type,name) \

#define AUTO_SER_PRIMITIVE_FIELD_PTR_WITH_GET(type,name) \
  AUTO_SER_PRIMITIVE_FIELD_PTR(type,name)\
  public:\
  const type *get##name() const { \
    return name;\
  }

#define AUTO_SER_PRIMITIVE_FIELD_PTR_WITH_SET(type,name) \
  AUTO_SER_PRIMITIVE_FIELD_PTR(type,name)\
  public:\
  void set##name(type *name) { \
    this->name = name;\
  }

#define AUTO_SER_PRIMITIVE_FIELD_PTR_WITH_GET_AND_SET(type,name) \
  AUTO_SER_PRIMITIVE_FIELD_PTR(type,name)\
  public:\
  const type *get##name() const { \
    return name;\
  }\
  void set##name(type *name) { \
    this->name = name;\
  }

#define AUTO_SER_PRIMITIVE_FIELD_PTR(type,name) \
  private:\
  type *name; \
  AUTO_SER_ENTRY_POINTS(name) \
  AUTO_SER_RETURN_SIZE_PRIMITIVE(name,type) \
  AUTO_SER_PRIMITIVE_FIELD_SERIALIZE_PTR(name) \
  AUTO_SER_PRIMITIVE_FIELD_DESERIALIZE_PTR(type,name) \


#define AUTO_SER_ENTRY_POINTS(name) \
  private:\
  template <typename T>\
  static NetMessage *serializeEntryPointMember##name(void* pthis,MyType<T> t,NetMessage *m, char *data, size_t &size) {\
    T* p = (T*) pthis;\
    return p->serializeMember##name(m, data, size);\
  }\

#define AUTO_SER_RETURN_SIZE_PRIMITIVE(name,type) \
  static size_t returnSizePrimitive##name(void) {\
    return sizeof(type);\
  }

#define AUTO_SER_RETURN_SIZE_PRIMITIVE_2(name,type1,type2) \
  static size_t returnSizePrimitive##name(void) {\
    return sizeof(type1,type2);\
  }

#define AUTO_SER_FIELD_SERIALIZE(name) \
   NetMessage *serializeMember##name(NetMessage *m, char *data, size_t &currentPos) { \
    if (m != NULL) { \
      return SerializationManager::getSerializationManager()->\
        serialize(name,m);\
    }\
    return m;\
  }

#define AUTO_SER_FIELD_DESERIALIZE(type,name) \
  template <typename T> \
  static void deserializeMember##name(T *c,\
    const NetMessage &data, size_t blockIndex, size_t &offset, size_t &netMessageIndex) { \
    const std::vector<NetMessage*> &messages = data.getMessages();\
    type* ser = (type*) SerializationManager::getSerializationManager()->deserialize(*(messages[netMessageIndex]),false);\
    ++netMessageIndex;\
    if (ser != (type*) NULL) {\
      c->name = *(type*)ser;\
      delete ser;\
    }\
  }\

#define AUTO_SER_FIELD_DESERIALIZE_2(type1,type2,name) \
  template <typename T> \
  static void deserializeMember##name(T *c,\
    const NetMessage &data, size_t blockIndex, size_t &offset, size_t &netMessageIndex) { \
    const std::vector<NetMessage*> &messages = data.getMessages();\
    type1,type2* ser = (type1,type2*) SerializationManager::getSerializationManager()->deserialize(*(messages[netMessageIndex]),false);\
    ++netMessageIndex;\
      if (ser != (type1,type2*) NULL) {\
        c->name = *(type1,type2*)ser;\
        delete ser;\
      }\
  }\

#define AUTO_SER_FIELD_DESERIALIZE_PTR(type,name) \
  template <typename T> \
  static void deserializeMember##name(T *c,\
    const NetMessage &data, size_t blockIndex, size_t &offset, size_t &netMessageIndex) { \
    const std::vector<NetMessage*> &messages = data.getMessages();\
    type* ser = (type*) SerializationManager::getSerializationManager()->deserialize(*(messages[netMessageIndex]),false);\
    ++netMessageIndex;\
    if (ser != (type*) NULL) {\
      c->name = (type*)ser;\
    }\
  }\

#define AUTO_SER_FIELD_DESERIALIZE_PTR_2(type1,type2,name) \
  template <typename T> \
  static void deserializeMember##name(T *c,\
    const NetMessage &data, size_t blockIndex, size_t &offset, size_t &netMessageIndex) { \
    const std::vector<NetMessage*> &messages = data.getMessages();\
    type1,type2* ser = (type1,type2*) SerializationManager::getSerializationManager()->deserialize(*(messages[netMessageIndex]),false);\
    ++netMessageIndex;\
    if (ser != (type1,type2*) NULL) {\
      c->name = (type1,type2*)ser;\
    }\
  }\

    
#define AUTO_SER_PRIMITIVE_FIELD_SERIALIZE(name) \
  NetMessage *serializeMember##name(NetMessage *m, char *data, size_t &currentPos) { \
    cvrtochars(name, &(data[currentPos]), currentPos);\
    return m;\
   }

#define AUTO_SER_PRIMITIVE_FIELD_SERIALIZE_PTR(name) \
  NetMessage *serializeMember##name(NetMessage *m, char *data, size_t &currentPos) { \
    cvrtochars(*name, &(data[currentPos]), currentPos);\
    return m;\
   }


#define AUTO_SER_CONVERT_uint8_t(x) convertToUInt8(x)
#define AUTO_SER_CONVERT_uint16_t(x) convertToUInt16(x)
#define AUTO_SER_CONVERT_uint32_t(x) convertToUInt32(x)
#define AUTO_SER_CONVERT_uint64_t(x) convertToUInt64(x)
#define AUTO_SER_CONVERT_int8_t(x) (uint8_t) convertToUInt8(x)
#define AUTO_SER_CONVERT_int16_t(x) (uint16_t) convertToUInt16(x)
#define AUTO_SER_CONVERT_int32_t(x) (uint32_t) convertToUInt32(x)
#define AUTO_SER_CONVERT_int64_t(x) (uint64_t) convertToUInt64(x)
#define AUTO_SER_CONVERT_float(x) convertToFloat(x)
#define AUTO_SER_CONVERT_double(x) convertToDouble(x)
#define AUTO_SER_CONVERT_char(x) (char) convertToUInt8(x)

#define AUTO_SER_PRIMITIVE_FIELD_DESERIALIZE(type,name) \
  template <typename T> \
  static void deserializeMember##name(T *c,\
    const NetMessage &data, size_t blockIndex, size_t &offset, size_t &netMessageIndex) { \
    c->name = (type) AUTO_SER_CONVERT_##type(data.getDataFromBlock(blockIndex,offset,sizeof(type)));\
  }\

#define AUTO_SER_PRIMITIVE_FIELD_DESERIALIZE_PTR(type,name) \
  template <typename T> \
  static void deserializeMember##name(T *c,\
    const NetMessage &data, size_t blockIndex, size_t &offset, size_t &netMessageIndex) { \
    type converted = (type) AUTO_SER_CONVERT_##type(data.getDataFromBlock(blockIndex,offset,sizeof(type)));\
    type *convertedPtr = new type;\
    *convertedPtr = converted;\
    c->name = convertedPtr;\
  }\

//! \def ITEM(name)
//! \brief Macro to use when passing args to AUTO_SER_STUFF().
//! \param name name of the field
#define ITEM(name) \
  (className::autoSerSerializeFunc) className::serializeEntryPointMember##name,\
  (className::autoSerDeserialiseFunc) className::deserializeMember##name,\
  (className::autoSerReturnSizePrimitive) className::returnSizePrimitive##name


//! \def AUTO_SER_STUFF(...)
//! \brief Add main serialization functions and set fields which will be
//! serialized.
//! \param ... list of fields (each one as arg of the ITEM(name) macro).
#define AUTO_SER_STUFF(...) \
  AUTO_SER_CLASS_OTHER_FUNCS \
  AUTO_SER_CLASS_INIT_MEMBERS(__VA_ARGS__)\
  AUTO_SER_CLASS_SER_FUNCS

//! \def AUTO_SER_STUFF_WITHOUT_MEMBERS
//! \brief Add main serialization functions
//! \param 
#define AUTO_SER_STUFF_WITHOUT_MEMBERS \
  AUTO_SER_CLASS_OTHER_FUNCS \
  AUTO_SER_CLASS_INIT_MEMBERS_EMPTY\
  AUTO_SER_CLASS_SER_FUNCS

//! \def AUTO_SER_STUFF_WITH_SUPERCLASS(superclass,...)
//! \brief Add main serialization functions for a subclass and set fields
//! which will be serialized
//! \param superclass the name of the superclass
//! \param ... list of fields (each one as arg of the ITEM(name) macro).
#define AUTO_SER_STUFF_WITH_SUPERCLASS(superclass,...) \
  AUTO_SER_CLASS_OTHER_FUNCS \
  AUTO_SER_CLASS_INIT_MEMBERS(__VA_ARGS__)\
  AUTO_SER_CLASS_SUPERCLASS_SER_FUNCS(superclass)

//! \def AUTO_SER_STUFF_WITH_SUPERCLASS_WITHOUT_MEMBERS(superclass)
//! \brief Add main serialization functions for a subclass
//! \param superclass the name of the superclass
#define AUTO_SER_STUFF_WITH_SUPERCLASS_WITHOUT_MEMBERS(superclass) \
  AUTO_SER_CLASS_OTHER_FUNCS \
  AUTO_SER_CLASS_INIT_MEMBERS_EMPTY\
  AUTO_SER_CLASS_SUPERCLASS_SER_FUNCS(superclass)

#define AUTO_SER_CLASS_OTHER_FUNCS \
  public:\
  typedef NetMessage* (*autoSerSerializeFunc) (void*,MyType<className>,NetMessage*, char *data, size_t &currentPos);\
  typedef void (*autoSerDeserialiseFunc) (className*,const NetMessage&, size_t blockIndex, size_t &offset, size_t &netMessageIndex);\
  typedef size_t (*autoSerReturnSizePrimitive) (void);\
  private:\
  MyType<className> classType;\
  static std::vector<autoSerSerializeFunc> serFuncs;\
  static std::vector<autoSerDeserialiseFunc> deserFuncs;\
  static size_t sizePrimitiveFields;\
  static void initMembers(int ph,...) {\
    va_list ap;\
    va_start(ap,ph);\
    void* v = va_arg(ap,void*);\
    int i = 0;\
    while (v != NULL) {\
      switch (i%3) {\
        case 0:\
          serFuncs.push_back((className::autoSerSerializeFunc) v);\
          break;\
        case 1:\
          deserFuncs.push_back((className::autoSerDeserialiseFunc) v);\
          break;\
        case 2:\
          sizePrimitiveFields += ((className::autoSerReturnSizePrimitive) v)();\
          break;\
      }\
      ++i;\
      v = va_arg(ap,void*);\
    }\
  }\
  static uint16_t type; \
  virtual uint16_t getType() const { \
    return type; \
  } \
  friend class libcomm;

#define AUTO_SER_CLASS_INIT_MEMBERS(...) \
  static void init() {\
    initMembers(0,__VA_ARGS__,NULL);\
  }

#define AUTO_SER_CLASS_INIT_MEMBERS_EMPTY \
  static void init() {\
  }

#define AUTO_SER_CLASS_SER_FUNCS \
  protected:\
  static Serializable *deserialize(const NetMessage &data, bool ptr) {\
    size_t blockIndex = 2;\
    size_t offset = 0;\
    size_t netMessageIndex = 0;\
    className *c = new className();\
    className **cptr = NULL; \
    if (ptr) cptr = new className*();\
    std::vector<autoSerDeserialiseFunc>::iterator it;\
    for(it = deserFuncs.begin();it != deserFuncs.end();++it){\
      (*it)(c,data,blockIndex,offset,netMessageIndex);\
    }\
    if (ptr) { \
      *cptr = c; \
      return (Serializable*) cptr; \
    } else { \
      return c; \
    }\
  }\
  void deserialize(const NetMessage &data, size_t &blockIndex, size_t &netMessageIndex) {\
    size_t offset = 0;\
    std::vector<autoSerDeserialiseFunc>::iterator it;\
    for(it=deserFuncs.begin();it != deserFuncs.end();++it){\
      (*it)(this,data,blockIndex,offset,netMessageIndex);\
    }\
    blockIndex += 2;\
  }\
  virtual NetMessage *serialize() const {\
    char *data = NULL;\
    size_t currentPos = 0;\
    if (sizePrimitiveFields != 0) data = (char*) malloc(sizePrimitiveFields);\
    NetMessage *message = new NetMessage(getType());\
    std::vector<autoSerSerializeFunc>::iterator it = serFuncs.begin();\
    for(;it != serFuncs.end(); ++it){\
      message = (*it)((void*)this,classType,message,data,currentPos);\
    }\
    if (sizePrimitiveFields != 0) message->addDataBlock(data, currentPos, true);\
    return message;\
  }

#define AUTO_SER_CLASS_SUPERCLASS_SER_FUNCS(superclass) \
  protected:\
  static Serializable *deserialize(const NetMessage &data, bool ptr) {\
    size_t blockIndex = 2;\
    size_t netMessageIndex = 0;\
    size_t offset = 0;\
    className *c = new className();\
    c->superclass::deserialize(data, blockIndex, netMessageIndex);\
    className **cptr = NULL; \
    if (ptr) cptr = new className*();\
    std::vector<autoSerDeserialiseFunc>::iterator it;\
    for(it = deserFuncs.begin();it != deserFuncs.end();++it){\
      (*it)(c,data,blockIndex, offset,netMessageIndex);\
    }\
    if (ptr) { \
      *cptr = c; \
      return (Serializable*) cptr; \
    } else { \
      return c; \
    }\
  }\
  void deserialize(const NetMessage &data, size_t &blockIndex, size_t &netMessageIndex) {\
    size_t offset = 0;\
    superclass::deserialize(data, blockIndex, netMessageIndex);\
    std::vector<autoSerDeserialiseFunc>::iterator it;\
    for(it=deserFuncs.begin();it != deserFuncs.end();++it){\
      (*it)(this,data,blockIndex,offset,netMessageIndex);\
    }\
    blockIndex += 2;\
  }\
  virtual NetMessage *serialize() const {\
    char *data = NULL;\
    size_t currentPos = 0;\
    if (sizePrimitiveFields != 0) data = (char*) malloc(sizePrimitiveFields);\
    NetMessage *message = superclass::serialize();\
    std::vector<autoSerSerializeFunc>::iterator it = serFuncs.begin();\
    for(;it != serFuncs.end(); ++it){\
      message = (*it)((void*)this,classType,message,data,currentPos);\
    }\
    if (sizePrimitiveFields != 0) message->addDataBlock(data, currentPos, true);\
    return message;\
  }


#define AUTO_SER_STATIC_STUFF \
  uint16_t className::type = 0;\
  std::vector<className::autoSerSerializeFunc> className::serFuncs;\
  std::vector<className::autoSerDeserialiseFunc> className::deserFuncs;\
  size_t className::sizePrimitiveFields = 0;

#endif
