#ifndef LIBCOMM_H
#define LIBCOMM_H

#include "serialization_manager.h"
#include <typeinfo>
#include <signal.h>

template <typename T>
class MyType {
};

typedef void (*SigAction) (int, siginfo_t*, void*); 

class libcomm {
  
  public : 
    static void init();
    static void clean();  
    template <typename T>
    static void addSupportFor(MyType<T> type) {
      SerializationManager *serManager = SerializationManager::getSerializationManager(); 
      T::type = serManager->addDeserializationFunc(&T::deserialize);
    }
    
    template <typename T>
    static void addSupportForAutoSerializable(MyType<T> type) {
      SerializationManager *serManager = SerializationManager::getSerializationManager(); 
      T::type = serManager->addDeserializationFunc(&T::deserialize);
      T::init();
    }
    static void setSigPipeHandler(SigAction handler);
};

#endif
