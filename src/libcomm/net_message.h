//! \file net_message.h
//! \brief Network message container
//! \author Alexandre Roux
//! \version 0.1
//! \date 05 august 2009
//!
//! File containing the declarations of the class NetMessage.

#ifndef NET_MESSAGE_H
#define NET_MESSAGE_H

#include <stdlib.h>
#include <sys/uio.h>
#include <stdint.h>
#include <string.h>
#include <vector>

#define UDP_PACKET_MAX_SIZE 65535
#define ETHER_IP_HEADERS 42 //ahem!

typedef  struct {
  char  *iov_base;    /* Starting address */
  size_t iov_len;     /* Number of bytes to transfer */
} chariovec;

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
//! The array of chars that is sent over the network, file_system, etc. has the 
//! following structure:
//! [_|__|_____|________...[_|__|____|___...][_|__|____|__...]...]
//!  1 2    3      4       5                 6
//! whith:                                         _
//!   1: one byte containing some flags             |
//!   2: two bytes defining the type of object      |-> Headers
//!   3: one up to eigth bytes for the size of data_|
//!   --                                           _
//!   4: data                                       |
//!   5: Inside data, another Netmessage serialized |-> Content
//!   6: Same as 5                                 _|
//!
//! Flags has the three weak bits used to determine the size of the 
//! size header
class NetMessage {
  
  private :
    uint16_t type;
    size_t dataSize;
    
    // Content for primitive type
    char *primitiveValue;
    size_t primitiveSize;

    // Data blocks
    chariovec *data;
    int iovCount;
    int iovAllocated;
    // Should data block bee freed?
    bool *freeIov;
    //size_t *ptrIov;
    
    // Nested NetMessages
    std::vector<NetMessage*> nestedNetMessages;
    // True if NetMessage headers have been generated
    bool headersGenerated;
    
    // Init NetMessages variables to NULL,0,false
    void initVars(); 
    // Find the size needed to represent the value of var
    size_t findRealSize(size_t var);
    // Prepare data and freeIov to add iovcnt elements.
    void addIovec(size_t iovcnt);
      
    // If type < 0, dataBlock, otherwise, NetMessage
    size_t generateBlockHeader(int type, size_t size, bool toFree, chariovec &iov);
    size_t generatePrimitiveBlockHeader(chariovec &iov);
    // Call generateBlockHeader and set headersGenerated to true
    void generateNetMessageHeader(void);
    
    // Collect iov recursively for the getData method
    void collectIov(chariovec **iov,  int *size);

  public :

    //! \brief NetMessage constructor
    //!
    //! Creates a new totally empty NetMessage.
    NetMessage(void);

    //! \brief NetMessage constructor
    //! \param[in] type the type of the Serializable object
    //!
    //! Creates a new NetMessage of type type. This constructor is used to
    //! create an instance of NetMessage without any data but a already known
    //! type
    NetMessage(uint16_t type);
    
    //! \brief NetMessage constructor for primitive type
    //! \param[in] type the type of the primitive type
    //! \param[in] value the primitive value in chars
    //! \param[in] primitiveSize the size of the primitive value, in bytes
    //!
    //! Creates a new NetMessage for a primitive type .
    NetMessage(uint16_t type, char *value, size_t primitiveSize);

    //! \brief NetMessage copy constructor
    //! \param[in] the constructor 
    //!
    //! Create a copy of the NetMessage given as argument.
    NetMessage(const NetMessage &net_message);
      
    //! \brief NetMessage destructor
    //!
    //! Deletes the current NetMessage. The destructor also deletes all the nested
    //! NetMessages 
    ~NetMessage(void);

    //! \brief Gets the type
    //! \return the type
    //!
    //! Returns the type of the serialized object contained in this NetMessage.
    uint16_t getType(void) const;
    
    //! \brief Gets the size of data
    //! \return the size of data
    //!
    //! Returns the size of the data representing the object, with or without 
    //! headers in bytes.
    uint32_t getDataSize(bool withHeaders);
    
    //! \brief Gets data
    //! \param[out] iovcnt The number of buffers
    //! \return the data
    //!
    //! This methods returns the data containing in this NetMessage and in its
    //! nested NetMessages.
    //! The data is returned in the form of a list of buffers. This buffers
    //! can be passed to the writev function. Therefore, it could be used to
    //! get the raw data to transfer on a stream. Note that the list must be
    //! freed with free.
    chariovec *getData(int *iovcnt);
    
    //! \brief Gets data block of the NetMessage
    //! \param[out] iovcnt The number of buffers
    //! \return the data
    //!
    //! This methods returns the data block associated with this NetMessage
    //! only (and not with nested NetMessage). Should be used to reconstruct
    //! an object from a NetMessage. Note that only buffers containing data are
    //! returned. The list must be freed with free.
    chariovec *getDataBlocks(int *iovcnt = NULL) const;
    
    //! \brief Gets nested NetMessages
    //! \return the messages
    //!
    //! Returns the NetMessages contained in this NetMessage
    const std::vector<NetMessage*> &getMessages(void) const;

    //! \brief Gets primitive value
    //! \return the primitive value
    //!
    //! Returns a pointer to the primitive value of this NetMessage.
    const char *getPrimitiveValue(void) const;

    
    // Delete all the data containing in this NetMessage, even those which 
    // correspoing freeIov bool is false.
    void deleteAllData(void);
    
    //! \brief Adds a NetMessage to the current NetMessage data array
    //! \param[in] message The NetMessage to add
    //!
    //! Appends data of the given NetMessage instance to the current
    //! NetMessage data array. Data size class variable is increased according
    //! to the number of bytes added. This methods is usefull for objects
    //! containing other objects.
    void addMessage(NetMessage *message);
    
    //! \brief Adds data to the current NetMessage data array.
    //! \param[in] newData the data to add
    //! \param[in] size the size of the new data
    //! \param[in] toFree true if data has to bee freed when NetMessage
    //!            is freed.
    //! \param[in] shouldgenerateBlockHeader true if the block headers have to be
    //!            generated. It should be true when the NetMessage is created
    //!            from an object, and false when it is created from raw data.
    //! \return the block id. Can be used with method appendData.
    //!
    //! Appends the given data block to the current NetMessage data array.
    int addDataBlock(char *newData, size_t size, bool toFree, 
      bool shouldgenerateBlockHeader = true);

    void appendDataToBlock(int blockId, const char *newData, size_t size);

    bool hasDataBlock(void);

    const char *getDataFromBlock(int blockId, size_t &offset, size_t size) const;

    //void setBlockAutoFree(int blockId, bool toFree);
    
    // Return the size of the flags header
    static size_t getFlagsHeaderSize(void);
    
    // Parse Flags. Returns a new NetMessage if the header corresponds to a
    // NetMessage, NULL otherwise. nextSize correspond to the remaining size of
    // the headers.
    NetMessage *parseFlags(const char *buff, size_t *nextSize, bool *toFree);
    // Parse the the remaining headers. If NetMessage, withType is true. blockSize
    // is filled with the size of the block.
    void parseTypeSize(const char *buff, bool withType, size_t sizeSize, size_t *blockSize);

};

#endif
