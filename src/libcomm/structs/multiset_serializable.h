//! \file multiset_serializable.h
//! \brief Serializable multiset
//! \author Alexandre Roux
//! \version 0.2
//! \date 20 august 2009
//!
//! File containing the declarations of the String object.

#ifndef MULTISET_H
#define MULTISET_H

#include <set>

#include "../serializable.h"
#include "../serialization_manager.h"

//! \class Multiset libcomm/structs/vector_serializable.h
//! \brief Serializable multiset
//!
//! This class is a subclass of the multiset class from the STL. Therefore, all
//! standard multiset functions are inherited. The class implements
//! the methods needed for the serialization.
template <typename K, typename C = std::less<K>, typename A = std::allocator<K> >
class Multiset : public std::multiset<K,C,A>, public Serializable {
  protected:
    static bool pointerContent;
    static uint16_t type;
    
    NetMessage *serialize() const;
    virtual uint16_t getType() const;
    static Serializable *deserialize(const NetMessage &data, bool ptr);

    friend class libcomm_structs;
    friend class libcomm;

  public:
    
    // std::multiset heritage
    //! \brief inherited
    Multiset(void);
    //! \brief inherited
    Multiset(const std::multiset<K,C,A> &c );
};

template <typename K, typename C, typename A>
uint16_t Multiset<K,C,A>::type = 0;
template <typename K, typename C, typename A>
bool Multiset<K,C,A>::pointerContent = false;

template <typename K, typename C, typename A>
uint16_t Multiset<K,C,A>::getType() const {
  return type; 
}

template <typename K, typename C, typename A>
NetMessage *Multiset<K,C,A>::serialize() const{

  SerializationManager *sM = SerializationManager::getSerializationManager();  
  NetMessage *message = new NetMessage(this->getType());
  typename std::multiset<K,C,A>::const_iterator iter = this->begin();
  for (;  iter != this->end(); ++iter) {
    message = sM->serialize(*iter, message);
  }
  return message;
}

#include <iostream>
template <typename K, typename C, typename A>
Serializable *Multiset<K,C,A>::deserialize(const NetMessage &data, bool ptr) {
  SerializationManager *sM = SerializationManager::getSerializationManager();  
  const std::vector<NetMessage*> &messages = data.getMessages();
  size_t multisetSize = messages.size();
  Multiset<K,C,A> *s = new Multiset<K,C,A>();

  for (size_t i = 0; i<multisetSize; ++i) {
    K *k = (K*) sM->deserialize(*(messages[i]), pointerContent);
    s->insert(*k);
    delete k;
  }
  if (ptr) {
    Multiset<K,C,A> **sPtr = new Multiset<K,C,A>*();
    *sPtr = s;
    return (Serializable*) sPtr;
  } else {
    return s;
  }
}

template <typename K, typename C, typename A>
Multiset<K,C,A>::Multiset(void) : std::multiset<K,C,A>() {}

template <typename K, typename C, typename A>
Multiset<K,C,A>::Multiset(const std::multiset<K,C,A> &c) : std::multiset<K,C,A>(c) {}

#endif
