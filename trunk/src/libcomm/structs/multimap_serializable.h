//! \file multimap_serializable.h
//! \brief Serializable multimap
//! \author Alexandre Roux
//! \version 0.2
//! \date 13 mars 2009
//!
//! File containing the declarations of the class Vector

#ifndef MULTIMAP_H
#define MULTIMAP_H

#include "../serialization_manager.h"
#include "../types_utils.h"
#include "../exception.h"

//! \class Multimap libcomm/structs/multimap_serializable.h
//! \brief Serializable multimap
//!
//! This class is a subclass of the multimap class from the STL. Therefore, all
//! standard multimap functions are inherited. The class implements
//! the methods needed for the serialization.
template <typename K, typename V, typename C = std::less<K>, 
          typename A  = std::allocator<std::pair<const K, V> > >
class Multimap : public std::multimap<K,V,C,A>, public Serializable {
  protected :
    static uint16_t type;
    static bool pointerContentKey;
    static bool pointerContentValue;

    NetMessage *serialize() const;
    static Serializable *deserialize(const NetMessage &data, bool ptr);
    virtual uint16_t getType() const;

    friend class libcomm_structs;
    friend class libcomm;
  public :
    // std::multimap heritage
    //! \brief inherited
    Multimap(void);
    //! \brief inherited
    Multimap(const std::multimap<K,V,C,A> &m);
    //! \brief inherited
    Multimap(  typename std::multimap<K,V,C,A>::iterator start,
          typename std::multimap<K,V,C,A>::iterator end);
    //! \brief inherited
    Multimap(  typename std::multimap<K,V,C,A>::iterator start,
          typename std::multimap<K,V,C,A>::iterator end,
          const C &cmp);
    //! \brief inherited
    Multimap(const C &cmp);

};

template <typename K, typename V, typename C, typename A>
uint16_t Multimap<K,V,C,A>::type = 0;
template <typename K, typename V, typename C, typename A>
bool Multimap<K,V,C,A>::pointerContentKey = false;
template <typename K, typename V, typename C, typename A>
bool Multimap<K,V,C,A>::pointerContentValue = false;

template <typename K, typename V, typename C, typename A>
uint16_t Multimap<K,V,C,A>::getType() const {
  return type;
}

template <typename K, typename V, typename C, typename A>
NetMessage *Multimap<K,V,C,A>::serialize() const {
  SerializationManager *sM = SerializationManager::getSerializationManager();  
  NetMessage *message = new NetMessage(this->getType());

  typename std::multimap<K,V,C,A>::const_iterator iter = this->begin();
  for (; iter != this->end(); ++iter) {
    message = sM->serialize(iter->first,message);
    message = sM->serialize(iter->second,message);
  }
  return message;
}

template <typename K, typename V, typename C, typename A>
Serializable *Multimap<K,V,C,A>::deserialize(const NetMessage &data, bool ptr) {
  SerializationManager *sm = SerializationManager::getSerializationManager();
  const std::vector<NetMessage*> &messages = data.getMessages();
  size_t multimapSize = messages.size() / 2;
  Multimap<K,V,C,A> *multimap = new Multimap<K,V,C,A>();

  
  for (uint16_t i(0); i<multimapSize; ++i) {
    K *k = (K*) sm->deserialize(*(messages[2*i]),pointerContentKey);
    V *v = (V*) sm->deserialize(*(messages[2*i+1]),pointerContentValue);
    multimap->insert(std::pair<K,V>(*k,*v));
    delete k;
    delete v;
  }
  if (ptr) {
    Multimap<K,V,C,A> **multimapPtr = new Multimap<K,V,C,A>*();
    *multimapPtr = multimap;
    return (Serializable*) multimapPtr;
  } else {
    return multimap;
  }
}


template <typename K, typename V, typename C, typename A>
Multimap<K,V,C,A>::Multimap() : std::multimap<K,V,C,A>() {}

template <typename K, typename V, typename C, typename A>
Multimap<K,V,C,A>::Multimap(const std::multimap<K,V,C,A> &m) : std::multimap<K,V,C,A>(m) {}

template <typename K, typename V, typename C, typename A>
Multimap<K,V,C,A>::Multimap(  typename std::multimap<K,V,C,A>::iterator start,
                    typename std::multimap<K,V,C,A>::iterator end)
  :  std::multimap<K,V,C,A>(start,end) {}

template <typename K, typename V, typename C, typename A>
Multimap<K,V,C,A>::Multimap(typename std::multimap<K,V,C,A>::iterator start, typename std::multimap<K,V,C,A>::iterator end, const C &cmp)
  :  std::multimap<K,V,C,A>(start,end,cmp) {}

template <typename K, typename V, typename C, typename A>
Multimap<K,V,C,A>::Multimap(const C &cmp) : std::multimap<K,V,C,A>(cmp) {}

#endif
