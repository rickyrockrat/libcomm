//! \file buffer_serializable.h
//! \brief Generic Serializable buffer
//! \author Alexandre Roux
//! \version 0.1
//! \date 13 mars 2009
//!
//! File containing the declaration and definition of a generic serializable
//! buffer.

#ifndef BUFFER_H
#define BUFFER_H

#include "../serialization_manager.h"
#include "../types_utils.h"
#include "../exception.h"
#include <errno.h>
#include <iostream>

#define BLOCK_SIZE 32

//! \class Buffer libcomm/structs/buffer_serializable.h
//! \brief Generic Serializable buffer
//!
//! This class implements a generic, extensible and serializable buffer that
//! can be used like a standard C array. Its size grows automatically when
//! new values are appended or when an element is accessed or written at a
//! greater index than the current buffer size. The buffer array is allocated
//! by blocks; the real allocated size of the buffer array is therefore greater
//! than or equal to the size used by its elements.
template <typename T>
class Buffer : public Serializable {
  private:
    size_t realSize;
    size_t allocatedSize;
    T *array;

    size_t computeNbBlocks(size_t size);

  protected:
    static uint16_t type;
    
    int returnDataSize() const;
    NetMessage *serialize() const;
    virtual uint16_t getType() const;
    static Serializable *deserialize(const NetMessage &data, bool ptr);

    friend class libcomm;
  public :
    static const size_t end;

    //! \brief Buffer constructor
    //!
    //! Creates a new Buffer of generic type T with a size of 0.
    Buffer();
    
    //! \brief Buffer constructor with initial size
    //! \param[in] size the initial size of the buffer.
    //!
    //! Creates a new Buffer of generic type T with a size equal to the given
    //! parameter.
    Buffer(size_t size);

    //! \brief Buffer destructor
    //!
    //! Deletes the current buffer object and frees its array.
    ~Buffer();
    
    //! \brief Gets the size of the buffer
    //! \return the size
    //!
    //! Returns the number of elements in the buffer.
    size_t size() const;

    //! \brief Gets the underlying buffer array
    //! \return the array
    //!
    //! Returns the array used by the buffer.
    const T *data() const;

    void set_data(T* data, size_t size);

    //! \brief Resizes the buffer
    //! \param[in] size the new size of the buffer.
    //!
    //! Resize the buffer to the new given size. If the new size is smaller than
    //! the previous one, the supernumerary elements are freed.
    void resize(size_t size);

    //! \brief Copies data in the buffer
    //! \param[in] start start index
    //! \param[in] src array to copy from
    //! \param[in] len number of elements to copy
    //!
    //! Copies elements from src at index start of length len to the buffer array.
    //! Note that len is the number of elements to copy, and not the size in bytes.
    void copyIn(size_t start, const T *src, size_t len);

    //! \brief Copies data out of the buffer.
    //! \param[in] start start index
    //! \param[out] dest array to copy to
    //! \param[in] len number of elements to copy
    //!
    //! Copies elements from buffer array at index start of length len to dest.
    //! Note that len is the number of elements to copy, and not the size in bytes.
    void copyTo(size_t start, T *dest, size_t len);

    //! \brief Operator []
    //! \param[in] index the index to access
    //! \return the element
    //!
    //! Accesses in read only the element at the given index. Note that if the index
    //! is greater than or equal to the allocated size, the buffer is resized.
    const T& operator[] (size_t index) const;

    //! \brief Operator []
    //! \param[in] index the index to access
    //! \return the element
    //!
    //! Accesses the element at the given index. Note that if the index is greater
    //! than or equal to the allocated size, the buffer is resized.
    T& operator[] (size_t index);

};

template <typename T>
const size_t Buffer<T>::end = -1;
template <typename T>
uint16_t Buffer<T>::type = 0;

template <typename T>
size_t Buffer<T>::computeNbBlocks(size_t size) {
  size_t mod;
  size_t nbBlocks;

  mod = (size % BLOCK_SIZE);
  nbBlocks = (size / BLOCK_SIZE) + ((mod) ? 1 : 0);
  return nbBlocks;
}

template <typename T>
Buffer<T>::Buffer(): realSize(0), allocatedSize(0), array(NULL) {}

template <typename T>
Buffer<T>::Buffer(size_t size): realSize(size) {
  if (size > 0) {
    allocatedSize = computeNbBlocks(size) * BLOCK_SIZE;
    array = (T*) malloc(allocatedSize*sizeof(T));
  } else {
    allocatedSize = realSize = 0;
    array = NULL;
  }
}

template <typename T>
Buffer<T>::~Buffer() {
  if (allocatedSize != 0) free(array);
}

template <typename T>
size_t Buffer<T>::size() const {
  return realSize;
}

template <typename T>
const T *Buffer<T>::data() const {
  return array;
}

template <typename T>
void Buffer<T>::set_data(T *data, size_t size) {
  if (allocatedSize != 0) free(array);
  allocatedSize = realSize = size;
  array = data;
}

template <typename T>
void Buffer<T>::resize(size_t size) {
  size_t newAllocatedSize;
  if (size < realSize) realSize = size;
  if (size > 0) {
    newAllocatedSize = computeNbBlocks(size) * BLOCK_SIZE;
    if (newAllocatedSize != allocatedSize) {
      allocatedSize = newAllocatedSize;
      array = (T*) realloc(array, allocatedSize*sizeof(T));
    }
  } else if (size == 0) {
    free(array);
    allocatedSize = realSize = 0;
  }
}

template <typename T>
void Buffer<T>::copyIn(size_t start, const T *src, size_t len) {
  if (start == Buffer<T>::end) {
    start = realSize;
    realSize = realSize+len;
  } else if ((start+len) > realSize) {
    realSize = start+len;
  }
  resize(realSize);

  memcpy(&(array[start]),src,len*sizeof(T));
}

template <typename T>
void Buffer<T>::copyTo(size_t start, T *dest, size_t len) {
  if (start+len > realSize) len = realSize;
  memcpy(dest,&(array[start]),len*sizeof(T));
}

template <typename T>
inline const T& Buffer<T>::operator[] (size_t index) const {
  if (index < realSize) {
    return array[index];
  } else {
    throw Exception(EFAULT, "Index greater than buffer size");
  }
}

template <typename T>
inline T& Buffer<T>::operator[] (size_t index) {
  if (index < realSize) {
    return array[index];
  } else if (index < allocatedSize) {
    realSize = index+1;
    return array[index];
  } else {
    resize(index+1);
    realSize = index+1;
    return (array[index]);
  }
}

template <typename T>
int Buffer<T>::returnDataSize() const {
  return (sizeof(uint64_t) + realSize*sizeof(T));
}

template <typename T>
uint16_t Buffer<T>::getType() const {
  return type; 
}

template <typename T>
NetMessage *Buffer<T>::serialize() const {
  size_t sizeSize = 0;
  char *c = convertToChars((uint64_t) this->size(), sizeSize);
  NetMessage *message = new NetMessage(getType(), c, sizeSize);
  if (message != NULL) {
    message->addData((char *) array,realSize*sizeof(T));
  }
  return message;
}

template <typename T>
Serializable *Buffer<T>::deserialize(const NetMessage &data, bool ptr) {
  Buffer<T> *buffer;

  const char *buff = data.getData();
  uint64_t size = convertToUInt64(buff);
  int currentPos = sizeof(uint64_t);
  
  buffer = new Buffer<T>(size);
  buffer->copyIn(0,(T*)&(buff[currentPos]),size);
  
  if (ptr) {
    Buffer<T> **bufferPtr = new Buffer<T>*();
    *bufferPtr = buffer;
    return (Serializable*) bufferPtr;
  } else {
    return buffer; 
  }
}

#endif
