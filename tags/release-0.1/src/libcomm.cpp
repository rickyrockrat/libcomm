#include "libcomm.h"
#include "types_utils.h"
#include "config_loader.h"
#include "logger.h"

#include "libcomm_structs.h"

void libcomm::init() {
  
  //types utils
  isLittleEndian();

  //Logger
  Logger::init();
  
  //Config loader
  ConfigLoader *cl = ConfigLoader::getConfigLoader();
  cl->loadParameters();

  //Initialisation stuff
  SerializationManager *serManager = SerializationManager::getSerializationManager(); 

  // Add builtin classes
  String::type = serManager->addDeserializationFunc(&String::deserialize);

  // Vector of chars
  Vector<char>::type = serManager->addDeserializationFunc(
                                  &Vector<char>::deserialize);
  // Vector of float
  Vector<float>::type = serManager->addDeserializationFunc( 
                                      &Vector<float>::deserialize);

  // Vector of double
  Vector<double>::type = serManager->addDeserializationFunc(
                                      &Vector<double>::deserialize);

  // Vector of int8_t
  Vector<int8_t>::type = serManager->addDeserializationFunc(
                                      &Vector<int8_t>::deserialize);
  
  // Vector of int16_t
  Vector<int16_t>::type = serManager->addDeserializationFunc(
                                      &Vector<int16_t>::deserialize);
  
  // Vector of int32_t
  Vector<int32_t>::type = serManager->addDeserializationFunc(
                                      &Vector<int32_t>::deserialize);
  
  // Vector of int64_t
  Vector<int64_t>::type = serManager->addDeserializationFunc(
                                      &Vector<int64_t>::deserialize);
  
  // Vector of uint8_t
  Vector<uint8_t>::type = serManager->addDeserializationFunc(
                                      &Vector<uint8_t>::deserialize);

  // Vector of uint16_t
  Vector<uint16_t>::type = serManager->addDeserializationFunc(
                                      &Vector<uint16_t>::deserialize);
  
  // Vector of uint8_t
  Vector<uint32_t>::type = serManager->addDeserializationFunc(
                                      &Vector<uint32_t>::deserialize);
  
  // Vector of uint8_t
  Vector<uint64_t>::type = serManager->addDeserializationFunc(
                                      &Vector<uint64_t>::deserialize);
  
  // Vector of strings
  Vector<String>::type = serManager->addDeserializationFunc(
                                      &Vector<String>::deserialize);

  // Vector of strings pointer
  Vector<String*>::type = serManager->addDeserializationFunc(
                                      &Vector<String*>::deserialize);
  Vector<String*>::pointerContent = true;
  
  //Buffer of chars
  Buffer<char>::type = serManager->addDeserializationFunc(
                                      &Buffer<char>::deserialize);
  //Buffer of int8_t
  Buffer<int8_t>::type = serManager->addDeserializationFunc(
                                      &Buffer<int8_t>::deserialize);
  //Buffer of int16_t
  Buffer<int16_t>::type = serManager->addDeserializationFunc(
                                      &Buffer<int16_t>::deserialize);
  //Buffer of int32_t
  Buffer<int32_t>::type = serManager->addDeserializationFunc(
                                      &Buffer<int32_t>::deserialize);
  //Buffer of int64_t
  Buffer<int64_t>::type = serManager->addDeserializationFunc(
                                      &Buffer<int64_t>::deserialize);
  //Buffer of uint8_t
  Buffer<uint8_t>::type = serManager->addDeserializationFunc(
                                      &Buffer<uint8_t>::deserialize);
  //Buffer of uint16_t
  Buffer<uint16_t>::type = serManager->addDeserializationFunc(
                                      &Buffer<uint16_t>::deserialize);
  //Buffer of uint32_t
  Buffer<uint32_t>::type = serManager->addDeserializationFunc(
                                      &Buffer<uint32_t>::deserialize);
  //Buffer of uint64_t
  Buffer<uint64_t>::type = serManager->addDeserializationFunc(
                                      &Buffer<uint64_t>::deserialize);
  //Map of char,String
  Map<char,String>::type = serManager->addDeserializationFunc(
                                      &Map<char,String>::deserialize);
  //Map of uint8_t,String
  Map<uint8_t,String>::type = serManager->addDeserializationFunc(
                                      &Map<uint8_t,String>::deserialize);
  //Map of uint16_t,String
  Map<uint16_t,String>::type = serManager->addDeserializationFunc(
                                      &Map<uint16_t,String>::deserialize);
  //Map of uint32_t,String
  Map<uint32_t,String>::type = serManager->addDeserializationFunc(
                                      &Map<uint32_t,String>::deserialize);
  //Map of int64_t,String
  Map<int64_t,String>::type = serManager->addDeserializationFunc(
                                      &Map<uint64_t,String>::deserialize);
  //Map of uint8_t,String
  Map<int8_t,String>::type = serManager->addDeserializationFunc(
                                      &Map<int8_t,String>::deserialize);
  //Map of int16_t,String
  Map<int16_t,String>::type = serManager->addDeserializationFunc(
                                      &Map<int16_t,String>::deserialize);
  //Map of int32_t,String
  Map<int32_t,String>::type = serManager->addDeserializationFunc(
                                      &Map<int32_t,String>::deserialize);
  //Map of int64_t,String
  Map<int64_t,String>::type = serManager->addDeserializationFunc(
                                      &Map<int64_t,String>::deserialize);
  //Map of char,String*
  Map<char,String*>::type = serManager->addDeserializationFunc(
                                      &Map<char,String*>::deserialize);
  Map<char,String*>::pointerContentValue = true;

  //Map of char,String*

  Map<String*,char,strPtrCmp>::type = serManager->addDeserializationFunc(
                                      &Map<String*,char,strPtrCmp>::deserialize);
  Map<String*,char,strPtrCmp>::pointerContentKey = true;
  
  //Map of char,String*
  Map<String*,String*,strPtrCmp>::type = serManager->addDeserializationFunc(
                                      &Map<String*,String*,strPtrCmp>::deserialize);
  Map<String*,String*,strPtrCmp>::pointerContentKey = true;
  Map<String*,String*,strPtrCmp>::pointerContentValue = true;
  
  //NullPlaceholder
  libcomm::addSupportForAutoSerializable(MyType<NullPlaceholder>());
}

void libcomm::clean() {

  //Cleanup stuff
  delete SerializationManager::getSerializationManager();
  delete ConfigLoader::getConfigLoader();
  Logger::cleanup();
}
