#ifndef LIBCOMM_STRUCTS_H
#define LIBCOMM_STRUCTS_H

#include "serialization_manager.h"
#include "structs/vector_serializable.h"
#include "structs/map_serializable.h"
#include "structs/multimap_serializable.h"
#include "structs/set_serializable.h"
#include "structs/multiset_serializable.h"
#include "structs/buffer_serializable.h"
#include "structs/simple_serializable.h"
#include "structs/string_serializable.h"
#include "structs/auto_serializable.h"
#include "structs/null_placeholder.h"

struct strPtrCmp {
  bool operator()( String* s1, String* s2 ) const {
    return s1->compare(*s2) < 0;
  }
};

class libcomm_structs {
  
  public :
    template <typename T>
    static void addSupportForVector(MyType<T> type, bool pointer) {
      SerializationManager *serManager = SerializationManager::getSerializationManager(); 
      Vector<T>::type = serManager->addDeserializationFunc(&Vector<T>::deserialize);
      Vector<T>::pointerContent = pointer;
    }

    template <typename T>
    static void addSupportForSet(MyType<T> type, bool pointer) {
      SerializationManager *serManager = SerializationManager::getSerializationManager(); 
      Set<T>::type = serManager->addDeserializationFunc(&Set<T>::deserialize);
      Set<T>::pointerContent = pointer;
    }

    template <typename T>
    static void addSupportForMultiset(MyType<T> type, bool pointer) {
      SerializationManager *serManager = SerializationManager::getSerializationManager(); 
      Multiset<T>::type = serManager->addDeserializationFunc(&Multiset<T>::deserialize);
      Multiset<T>::pointerContent = pointer;
    }

    template <typename K, typename V>
    static void addSupportForMap(MyType<K> type1, MyType<V> type2, bool pointerKey, bool pointerValue) {
      SerializationManager *serManager = SerializationManager::getSerializationManager(); 
      Map<K,V>::type = serManager->addDeserializationFunc(&Map<K,V>::deserialize);
      Map<K,V>::pointerContentKey = pointerKey;
      Map<K,V>::pointerContentValue = pointerValue;
    }

    template <typename K, typename V>
    static void addSupportForMultimap(MyType<K> type1, MyType<V> type2, bool pointerKey, bool pointerValue) {
      SerializationManager *serManager = SerializationManager::getSerializationManager(); 
      Multimap<K,V>::type = serManager->addDeserializationFunc(&Multimap<K,V>::deserialize);
      Multimap<K,V>::pointerContentKey = pointerKey;
      Multimap<K,V>::pointerContentValue = pointerValue;
    }
};

#endif
