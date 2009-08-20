//! \file set_serializable.h
//! \brief Serializable set
//! \author Alexandre Roux
//! \version 0.2
//! \date 20 august 2009
//!
//! File containing the declarations of the String object.

#ifndef SET_H
#define SET_H

#include <set>

#include "../serializable.h"
#include "../serialization_manager.h"

//! \class Set libcomm/structs/vector_serializable.h
//! \brief Serializable set
//!
//! This class is a subclass of the set class from the STL. Therefore, all
//! standard set functions are inherited. The class implements
//! the methods needed for the serialization.
template <typename K, typename C = std::less<K>, typename A = std::allocator<K> >
class Set : public std::set<K,C,A>, public Serializable {
  protected:
    static bool pointerContent;
    static uint16_t type;
    
    NetMessage *serialize() const;
    virtual uint16_t getType() const;
    static Serializable *deserialize(const NetMessage &data, bool ptr);

    friend class libcomm_structs;
    friend class libcomm;

  public:
    
    // std::set heritage
    //! \brief inherited
    Set(void);
    //! \brief inherited
    Set(const std::set<K,C,A> &c );
};

template <typename K, typename C, typename A>
uint16_t Set<K,C,A>::type = 0;
template <typename K, typename C, typename A>
bool Set<K,C,A>::pointerContent = false;

template <typename K, typename C, typename A>
uint16_t Set<K,C,A>::getType() const {
  return type; 
}

template <typename K, typename C, typename A>
NetMessage *Set<K,C,A>::serialize() const{

  SerializationManager *sM = SerializationManager::getSerializationManager();  
  NetMessage *message = new NetMessage(this->getType());
  typename std::set<K,C,A>::const_iterator iter = this->begin();
  for (;  iter != this->end(); ++iter) {
    message = sM->serialize(*iter, message);
  }
  return message;
}

#include <iostream>
template <typename K, typename C, typename A>
Serializable *Set<K,C,A>::deserialize(const NetMessage &data, bool ptr) {
  SerializationManager *sM = SerializationManager::getSerializationManager();  
  const std::vector<NetMessage*> &messages = data.getMessages();
  size_t setSize = messages.size();
  Set<K,C,A> *s = new Set<K,C,A>();

  for (size_t i = 0; i<setSize; ++i) {
    K *k = (K*) sM->deserialize(*(messages[i]), pointerContent);
    s->insert(*k);
    delete k;
  }
  if (ptr) {
    Set<K,C,A> **sPtr = new Set<K,C,A>*();
    *sPtr = s;
    return (Serializable*) sPtr;
  } else {
    return s;
  }
}

template <typename K, typename C, typename A>
Set<K,C,A>::Set(void) : std::set<K,C,A>() {}

template <typename K, typename C, typename A>
Set<K,C,A>::Set(const std::set<K,C,A> &c) : std::set<K,C,A>(c) {}

#endif
