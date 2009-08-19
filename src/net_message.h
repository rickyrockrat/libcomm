//! \file net_message.h
//! \brief Network message container
//! \author Alexandre Roux
//! \version 0.1
//! \date 13 mars 2009
//!
//! File containing the declarations of the class NetMessage.

#ifndef NET_MESSAGE_H
#define NET_MESSAGE_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define UDP_PACKET_MAX_SIZE 65535
#define ETHER_IP_HEADERS 42 //ahem!
#define NET_MESSG_NEDFH -1
#define NET_MESSG_NEDFC -2

//! \class NetMessage libcomm/net_message.h
//! \brief Network message container
//!
//! This class is a container for network messages. It contains the state of a
//! serializable object in the form of an array of bytes. The class provides 
//! constructors for creating network messages from arrays of bytes and 
//! methods for generating arrays of bytes from network messages instances.
//! NetMessages were initially designed to contain data to send to or receive 
//! from the network. Actually, there are generic enough to be used for 
//! different related tasks, like cloning objects and saving them
//! in files. Note that, except if you are doing very specific operations, you
//! should not need to manipulate directly this class. See AutoSerializable and
//! SimpleSerializable for automatic serialization. See SerializationManager
//! for serialization helpers.
class NetMessage {
  
  private :
    time_t timeStamp;
    uint16_t type;
    uint32_t dataSize;

    char *data;
    
    void setRawData(bool withTs, char *c) const;

  public :
    //! \brief NetMessage constructor
    //! \param[in] type the type of the Serializable object
    //! \param[in] data the state of the object, in bytes
    //! \param[in] dataSize the length of the data
    //!
    //! Creates a new NetMessage, assigning each parameter to its corresponding
    //! class variables and setting its timestamp to the current time. This
    //! constructor is used to construct a NetMessage from an object.
    NetMessage(uint16_t type, char *data, uint32_t dataSize);

    //! \brief NetMessage constructor
    //! \param[in] buffer the buffer of byte to read from
    //! \param[in] bufferAvailable the available size of data in the buffer
    //! \param[out] byteRead the number of bytes read from the buffer by the 
    //!             constructor
    //! \param[in] fetchTs true if the constructor has to fetch the timestamp
    //! \exception Exception if there is not enough data available for headers
    //!            (NET_MESSG_NEDFH) or for object content (NET_MESSG_NEDFC)
    //!
    //! Constructs a new NetMessage fom a buffer of bytes. The buffer is parsed
    //! and the type, the size of the object data, the timestamp (optionnaly) 
    //! and the object data are extracted. This constructor is used to construct
    //! a NetMessage from an array of bytes.
    NetMessage(const char *buffer, size_t bufferAvailable, size_t *byteRead, bool fetchTs);
    
    //! \brief NetMessage destructor
    //!
    //! Deletes the current NetMessage and frees its data.
    ~NetMessage();

    //! \brief Gets the type
    //! \return the type
    //!
    //! Returns the type of the serialized object contained in this NetMessage.
    uint16_t getType() const;
    void setType(uint16_t type);
    
    //! \brief Gets the size of data
    //! \return the size of data
    //!
    //! Returns the size of the data representing the object, without headers
    //! like type and timestamp, in bytes.
    uint32_t getDataSize() const;
    
    //! \brief Gets the timestamp
    //! \return the timestamp
    //!
    //! Returns the timestamp that corresponds to the time of serialization
    //! of the object containing in the NetMessage.
    time_t getTimeStamp() const;
    
    //! \brief Gets the underlying array of bytes
    //! \return the data
    //!
    //! Returns a read only pointer to the array of bytes encapsulated in the
    //! current NetMessage instance.
    const char *getData() const;
    
    //! \brief Gets a complete copy of the object state
    //! \return the data
    //!
    //! Returns a complete copy of the object state, including headers. This
    //! data is complete enough to reconstruct, from them, a copy of the 
    //! object contained in the current NetMessage instance. It is therefore
    //! used as raw data to send into sockets, to write in files or simply 
    //! to clone serializable objects.
    char *getRawData(bool withTs, size_t *size) const;

    //void addData(uint16_t type, const char *newData, size_t size);
    
    //! \brief Adds a NetMessage to the current NetMessage data array
    //! \param[in] message The NetMessage to add
    //!
    //! Appends raw data of the given NetMessage instance to the current
    //! NetMessage data array. Data size class variable is increased according
    //! to the number of bytes added. This methods is usefull for objects
    //! containing other objects.
    void addData(const NetMessage &message);
    
    //! \brief Adds data to the current NetMessage data array.
    //! \param[in] newData the data to add
    //! \param[in] size the size of the new data
    //!
    //! Appends the given data to the current NetMessage data array. Data size
    //! class variable is increased by the size parameter. This method is used
    //! to add some data that is not a complete object.
    void addData(const char *newData, size_t size);

};

#endif
