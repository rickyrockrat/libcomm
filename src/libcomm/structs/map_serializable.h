//! \file map_serializable.h
//! \brief Serializable map
//! \author Alexandre Roux
//! \version 0.2
//! \date 13 mars 2009
//!
//! File containing the declarations of the class Vector

#ifndef MAP_H
#define MAP_H

#include "../serialization_manager.h"
#include "../types_utils.h"
#include "../exception.h"

//! \class Map libcomm/structs/map_serializable.h
//! \brief Serializable map
//!
//! This class is a subclass of the map class from the STL. Therefore, all
//! standard map functions are inherited. The class implements
//! the methods needed for the serialization.
template <typename K, typename V, typename C = std::less<K>, 
          typename A  = std::allocator<std::pair<const K, V> > >
class Map : public std::map<K,V,C,A>, public Serializable {
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
    // std::map heritage
    //! \brief inherited
    Map(void);
    //! \brief inherited
    Map(const std::map<K,V,C,A> &m);
    //! \brief inherited
    Map(  typename std::map<K,V,C,A>::iterator start,
          typename std::map<K,V,C,A>::iterator end);
    //! \brief inherited
    Map(  typename std::map<K,V,C,A>::iterator start,
          typename std::map<K,V,C,A>::iterator end,
          const C &cmp);
    //! \brief inherited
    Map(const C &cmp);

};

template <typename K, typename V, typename C, typename A>
uint16_t Map<K,V,C,A>::type = 0;
template <typename K, typename V, typename C, typename A>
bool Map<K,V,C,A>::pointerContentKey = false;
template <typename K, typename V, typename C, typename A>
bool Map<K,V,C,A>::pointerContentValue = false;

template <typename K, typename V, typename C, typename A>
uint16_t Map<K,V,C,A>::getType() const {
  return type;
}

template <typename K, typename V, typename C, typename A>
NetMessage *Map<K,V,C,A>::serialize() const {
  SerializationManager *sM = SerializationManager::getSerializationManager();  
  NetMessage *message = new NetMessage(this->getType());

  typename std::map<K,V,C,A>::const_iterator iter = this->begin();
  for (; iter != this->end(); ++iter) {
    message = sM->serialize(iter->first,message);
    message = sM->serialize(iter->second,message);
  }
  return message;
}

template <typename K, typename V, typename C, typename A>
Serializable *Map<K,V,C,A>::deserialize(const NetMessage &data, bool ptr) {
  SerializationManager *sm = SerializationManager::getSerializationManager();
  const std::vector<NetMessage*> &messages = data.getMessages();
  size_t mapSize = messages.size() / 2;
  Map<K,V,C,A> *map = new Map<K,V,C,A>();

  
  for (uint16_t i(0); i<mapSize; ++i) {
    K *k = (K*) sm->deserialize(*(messages[2*i]),pointerContentKey);
    V *v = (V*) sm->deserialize(*(messages[2*i+1]),pointerContentValue);
    (*map)[*k] = *v;
    delete k;
    delete v;
  }
  if (ptr) {
    Map<K,V,C,A> **mapPtr = new Map<K,V,C,A>*();
    *mapPtr = map;
    return (Serializable*) mapPtr;
  } else {
    return map;
  }
}


template <typename K, typename V, typename C, typename A>
Map<K,V,C,A>::Map() : std::map<K,V,C,A>() {}

template <typename K, typename V, typename C, typename A>
Map<K,V,C,A>::Map(const std::map<K,V,C,A> &m) : std::map<K,V,C,A>(m) {}

template <typename K, typename V, typename C, typename A>
Map<K,V,C,A>::Map(  typename std::map<K,V,C,A>::iterator start,
                    typename std::map<K,V,C,A>::iterator end)
  :  std::map<K,V,C,A>(start,end) {}

template <typename K, typename V, typename C, typename A>
Map<K,V,C,A>::Map(typename std::map<K,V,C,A>::iterator start, typename std::map<K,V,C,A>::iterator end, const C &cmp)
  :  std::map<K,V,C,A>(start,end,cmp) {}

template <typename K, typename V, typename C, typename A>
Map<K,V,C,A>::Map(const C &cmp) : std::map<K,V,C,A>(cmp) {}

#endif
