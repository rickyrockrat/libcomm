//! \file serializable.h
//! \brief Serializable data structure
//! \author Alexandre Roux
//! \version 0.1
//! \date 13 mars 2009
//!
//! File containing the declaration of the class Serializable.
#ifndef SERIALIZABLE_H
#define SERIALIZABLE_H

#include "net_message.h"

//! \class Serializable libcomm/serializable.h
//! \brief Superclass of all serializable classes
//!
//! This class declares the methods that a serializable class must provide.
class Serializable {
 
  private :
    friend class SerializationManager;
  
  protected :

    //! \brief Returns the type id. of the Serializable object
    //! \return the type id.
    //!
    //! Each Serializable class must have a unique type id. It is needed by
    //! the SerializationManager to know which deserialization function to call
    //! when deserializing a NetMessage.
    virtual uint16_t getType(void) const = 0;
    
    //! \brief Serializes the object
    //! \return a pointer to the generated NetMessage
    //!
    //! Serializes the current serializable instance, returning a NetMessage
    //! containing the object state.
    virtual NetMessage *serialize(void) const = 0;

    //! \brief Deserializes a NetMessage
    //! \param[in] data the NetMessage to deserialize
    //! \param[in] ptr if true, the returned Serializable object pointer must be in
    //! reality a pointer with one level of indirection to the object
    //! (so, with a real type of Serializable**).
    //! \return a pointer to the generated Serializable object or NULL on error
    //!
    //! Deserializes the given NetMessage into a Serializable object. The ptr
    //! parameter is used by data structures like Vectors that can
    //! contain objects (for instance: Vector<String>) or pointers to objects
    //! (for instance: Vector<String*>).
    static Serializable *deserialize(const NetMessage &data, bool ptr);

  public :
    //! \brief Serializable destructor
    virtual ~Serializable(void);

};

typedef Serializable* (*DeserializeFunc) (const NetMessage &data, bool ptr);

#endif
