//! \file serialization_manager.h
//! \brief Serialization manager
//! \author Alexandre Roux
//! \version 0.1
//! \date 13 mars 2009
//!
//! File containing the declaration of the class SerializationManager.
#ifndef SERIALIZATION_MANAGER_H
#define SERIALIZATION_MANAGER_H

#include <map>

#include "serializable.h"

//! \class SerializationManager libcomm/serialization_manager.h
//! \brief Serialization manager
//!
//! This class manages objects serialization. It provides several methods
//! to ease serialization of data and is responsible for calling the correct
//! deserialization methods. This class implements the singleton design pattern.
//!
//! If you decide not to use neither the AutoSerializable nor the
//! SimpleSerializable class, you will need to implement yourself the different
//! serialization methods. This class provides several methods that you should
//! use to make it.
class SerializationManager {

  private :
    std::map<uint16_t, DeserializeFunc> deserializeFuncs;
    uint16_t currentId;

    SerializationManager(void);
    static SerializationManager *self;
    NetMessage *mergeMessage(NetMessage *message, NetMessage *newMessage) const;
  
  public :

    //! \brief Returns the unique SerializationManager instance.
    //! \return the SerializationManager instance
    //!
    //! Since the SerializationManager class implements the singleton design
    //! pattern, there must be only one SerializationManager instance. This
    //! method ensures this requirement; call to get the unique instance of
    //! the SerializationManager.
    static SerializationManager *getSerializationManager(void);
    
    //used by libcomm class only
    void addDeserializationFunc(const uint16_t type, DeserializeFunc f);     
    //used by libcomm class only
    uint16_t addDeserializationFunc(DeserializeFunc f);     
    
    //! \brief Clones the given Serializable object
    //! \param object the Serializable object to clone
    //! \return the new Serializable object
    //!
    //! Clones a Serializable object. The methods serializes the object
    //! into a NetMessage which is then deserialized in a new, identical, 
    //! Serializable object.
    Serializable *clone(const Serializable &object);

    //! \brief Clones the given Serializable object
    //! \param object the Serializable pointer to the object to clone
    //! \return the new Serializable object
    //!
    //! Clones a Serializable object. The methods serializes the object
    //! into a NetMessage which is then deserialized in a new, identical, 
    //! Serializable object.
    Serializable *clone(const Serializable *object);
    
    //! \brief Deserializes a NetMessage into an object or a primitive value
    //! \param[in] message the NetMessage to deserialize from
    //! \param[in] ptr boolean passed to the deserialize method
    //! (see Serializable::deserialize())
    //! \return the value deserialized or NULL if an error occurs
    //!
    //! Returns a newly created pointer on the value deserialized from the
    //! given NetMessage. The value can be either a primitive value or an object.
    //! If ptr is true, the returned pointer points to a pointer to the value:
    //! For a serializable object, it will return Serializable**.
    void *deserialize(const NetMessage &message, bool ptr);
    
    //! \brief Deserializes a NetMessage into an object or a primitive value
    //! \param[in] message the NetMessage pointer to deserialize from
    //! \param[in] ptr boolean passed to the deserialize method
    //! (see Serializable::deserialize())
    //! \return the value deserialized or NULL if an error occurs
    //!
    //! Returns a newly created pointer on the value deserialized from the
    //! given NetMessage. The value can be either a primitive value or an object.
    //! If ptr is true, the returned pointer points to a pointer to the value:
    //! For a serializable object, it will return Serializable**.
    void *deserialize(const NetMessage *message, bool ptr);

    // deserialize primitive. Used by libcomm class only
    void *deserializePrimitive(const NetMessage &message);
    
    //! \brief Serializes an object
    //! \param[in] object the object to serialize
    //! \param[in] message the message to be merged with the generated
    //! NetMessage containing the object serialization, or NULL if no merge
    //! is needed
    //! \return a new NetMessage containing the object serialization and, if not
    //! NULL, the parameter message.
    //!
    //! Calls the serialize method on the object. If the message parameter is
    //! not NULL, a merge is done between it and the NetMessage generated from
    //! the object parameter. The second one is added to the first one using
    //! the NetMessage::addData method. This method is usefull to add several
    //! objects to the same NetMessage.
    NetMessage *serialize(const Serializable &object, NetMessage *message) const;

    //! \brief Serializes an object
    //! \param[in] object the object to serialize
    //! \param[in] message messate to add, or NULL for none
    //! \return a new NetMessage
    //!
    //! Similar to SerializationManager::serialize, but with a pointer to an 
    //! object instead of a reference.
    NetMessage *serialize(const Serializable *object, NetMessage *message) const;
    
    //! \brief Serializes an char
    //! \param[in] c the char to serialize
    //! \param[in] message messate to add, or NULL for none
    //! \return a new NetMessage
    //!
    //! Similar to SerializationManager::serialize, but with a char instead
    //! of an objet reference
    NetMessage *serialize(char c, NetMessage *message) const;
    
    //! \brief Serializes a float
    //! \param[in] f the float to serialize
    //! \param[in] message messate to add, or NULL for none
    //! \return a new NetMessage
    //!
    //! Similar to SerializationManager::serialize, but with a float instead
    //! of an objet reference
    NetMessage *serialize(float f, NetMessage *message) const;
    
    //! \brief Serializes a double
    //! \param[in] d the double to serialize
    //! \param[in] message messate to add, or NULL for none
    //! \return a new NetMessage
    //!
    //! Similar to SerializationManager::serialize, but with a double instead
    //! of an objet reference
    NetMessage *serialize(double d, NetMessage *message) const;
    
    //! \brief Serializes an int8_t
    //! \param[in] i the int8_t the char to serialize
    //! \param[in] message messate to add, or NULL for none
    //! \return a new NetMessage
    //!
    //! Similar to SerializationManager::serialize, but with an int8_t instead
    //! of an objet reference
    NetMessage *serialize(int8_t i, NetMessage *message) const;
    
    //! \brief Serializes an int16_t
    //! \param[in] i the int16_t to serialize
    //! \param[in] message messate to add, or NULL for none
    //! \return a new NetMessage
    //!
    //! Similar to SerializationManager::serialize, but with an int16_t instead
    //! of an objet reference
    NetMessage *serialize(int16_t i, NetMessage *message) const;
    
    //! \brief Serializes an int32_t
    //! \param[in] i the int32_t to serialize
    //! \param[in] message messate to add, or NULL for none
    //! \return a new NetMessage
    //!
    //! Similar to SerializationManager::serialize, but with an int32_t instead
    //! of an objet reference
    NetMessage *serialize(int32_t i, NetMessage *message) const;
    
    //! \brief Serializes an int64_t
    //! \param[in] i the int64_t to serialize
    //! \param[in] message messate to add, or NULL for none
    //! \return a new NetMessage
    //!
    //! Similar to SerializationManager::serialize, but with an int364t instead
    //! of an objet reference
    NetMessage *serialize(int64_t i, NetMessage *message) const;
    
    //! \brief Serializes an uint8_t
    //! \param[in] i the uint8_t to serialize
    //! \param[in] message messate to add, or NULL for none
    //! \return a new NetMessage
    //!
    //! Similar to SerializationManager::serialize, but with an uint8_t instead
    //! of an objet reference
    NetMessage *serialize(uint8_t i, NetMessage *message) const;
    
    //! \brief Serializes an uint16_t
    //! \param[in] i the uint16_t to serialize
    //! \param[in] message messate to add, or NULL for none
    //! \return a new NetMessage
    //!
    //! Similar to SerializationManager::serialize, but with an uint16_t instead
    //! of an objet reference
    NetMessage *serialize(uint16_t i, NetMessage *message) const;
    
    //! \brief Serializes an uint32_t
    //! \param[in] i the uint32_t to serialize
    //! \param[in] message messate to add, or NULL for none
    //! \return a new NetMessage
    //!
    //! Similar to SerializationManager::serialize, but with an uint32_t instead
    //! of an objet reference
    NetMessage *serialize(uint32_t i, NetMessage *message) const;
    
    //! \brief Serializes an uint64_t
    //! \param[in] i the uint64_t to serialize
    //! \param[in] message messate to add, or NULL for none
    //! \return a new NetMessage
    //!
    //! Similar to SerializationManager::serialize, but with an uint64_t instead
    //! of an objet reference
    NetMessage *serialize(uint64_t i, NetMessage *message) const;

    void deleteT(Serializable &object) const;
    void deleteT(Serializable *object) const;
    void deleteT(char c) const;
    void deleteT(float f) const;
    void deleteT(double d) const;
    void deleteT(int8_t i) const;
    void deleteT(int16_t i) const;
    void deleteT(int32_t i) const;
    void deleteT(int64_t i) const;
    void deleteT(uint8_t i) const;
    void deleteT(uint16_t i) const;
    void deleteT(uint32_t i) const;
    void deleteT(uint64_t i) const;

    
    ~SerializationManager(void);

};

#endif
