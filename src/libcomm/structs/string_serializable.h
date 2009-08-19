//! \file string_serializable.h
//! \brief Serializable string
//! \author Alexandre Roux
//! \version 0.1
//! \date 13 mars 2009
//!
//! File containing the declarations of the String object.

#ifndef STRING_H
#define STRING_H

#include <string>

#include "../serializable.h"

//! \class String libcomm/structs/string_serializable.h
//! \brief Serializable string
//!
//! This class is a subclass of the string class from the STL. Therefore, all
//! standard string functions are inherited. The class implements
//! the methods needed for the serialization.
class String : public Serializable, public std::string {
  
  protected :
    static uint16_t type;

    int returnDataSize() const;
    NetMessage *serialize() const;
    static Serializable *deserialize(const NetMessage &data, bool ptr);
    virtual uint16_t getType() const;

    friend class libcomm;
    
  public :
    
    // std::string heritage
    //! \brief inherited
    String(void);
    //! \brief inherited
    String(const std::string &s);
    //! \brief inherited
    String(size_t length, const char &ch);
    //! \brief inherited
    String(const char *str);
    //! \brief inherited
    String(const char *str, size_t length);
    //! \brief inherited
    String(const std::string &str, size_t index, size_t length);
    //! \brief inherited
    String(std::string::iterator start, std::string::iterator end);
};

#endif
