//! \file vector_serializable.h
//! \brief Serializable vector
//! \author Alexandre Roux
//! \version 0.1
//! \date 13 mars 2009
//!
//! File containing the declarations of the class Vector

#ifndef VECTOR_H
#define VECTOR_H

#include <vector>

#include "../serialization_manager.h"
#include "../types_utils.h"
#include "../exception.h"

//! \class Vector libcomm/structs/vector_serializable.h
//! \brief Serializable vector
//!
//! This class is a subclass of the vector class from the STL. Therefore, all
//! standard vector functions are inherited. The class implements
//! the methods needed for the serialization.
template <typename T, typename A = std::allocator<T> >
class Vector : public std::vector<T,A>, public Serializable {
  
  protected:
    static bool pointerContent;
    static uint16_t type;
    
    NetMessage *serialize() const;
    virtual uint16_t getType() const;
    static Serializable *deserialize(const NetMessage &data, bool ptr);

    friend class libcomm_structs;
    friend class libcomm;
  public :
    //std::vector heritage
    //! \brief inherited
    Vector(void);
    //! \brief inherited
    Vector(const std::vector<T,A> &c);
    //! \brief inherited
    Vector(size_t num, const T &val);
    //! \brief inherited
    Vector( typename std::vector<T,A>::iterator start,
            typename std::vector<T,A>::iterator end);
};

template <typename T, typename A>
uint16_t Vector<T,A>::type = 0;
template <typename T, typename A>
bool Vector<T,A>::pointerContent = false;

template <typename T, typename A>
uint16_t Vector<T,A>::getType() const {
  return type; 
}

template <typename T, typename A>
NetMessage *Vector<T,A>::serialize() const{

  SerializationManager *sM = SerializationManager::getSerializationManager();  
  NetMessage *message = new NetMessage(this->getType());
  typename std::vector<T,A>::const_iterator iter = this->begin();
  for (;  iter != this->end(); ++iter) {
    message = sM->serialize(*iter, message);
  }
  return message;
}

#include <iostream>
template <typename T, typename A>
Serializable *Vector<T,A>::deserialize(const NetMessage &data, bool ptr) {
  SerializationManager *sM = SerializationManager::getSerializationManager();  
  const std::vector<NetMessage*> &messages = data.getMessages();
  size_t vectorSize = messages.size();
  Vector<T,A> *v = new Vector<T,A>(vectorSize);

  for (size_t i = 0; i<vectorSize; ++i) {
    T *t = (T*) sM->deserialize(*(messages[i]), pointerContent);
    (*v)[i] = *t;
    delete t;
  }
  if (ptr) {
    Vector<T,A> **vPtr = new Vector<T,A>*();
    *vPtr = v;
    return (Serializable*) vPtr;
  } else {
    return v;
  }
}

template <typename T, typename A>
Vector<T,A>::Vector() : std::vector<T,A>() {}

template <typename T, typename A>
Vector<T,A>::Vector(const std::vector<T,A> &c) : std::vector<T,A>(c) {}

template <typename T, typename A>
Vector<T,A>::Vector(size_t num, const T &val = T()) 
                    : std::vector<T,A>(num,val) {}

template <typename T, typename A>
Vector<T,A>::Vector(  typename std::vector<T,A>::iterator start,
                      typename std::vector<T,A>::iterator end) 
                    : std::vector<T,A>(start,end) {}

#endif
