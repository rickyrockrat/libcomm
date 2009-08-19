//! \file null_placeholder.h
//! \brief NULL placeholder
//! \author Alexandre Roux
//! \version 0.1
//! \date 13 mars 2009
//!
//! File containing the declarations of the class NullPlaceholder

#ifndef NULL_PLACEHOLDER_H
#define NULL_PLACEHOLDER_H

#include "auto_serializable.h"

#define className NullPlaceholder

//! \class NullPlaceholder libcomm/structs/null_placeholder.h
//! \brief NULL placeholder
//!
//! This class is a placeholder for the NULL value. It is a Serializable object
//! which doesn't contain any field.
class NullPlaceholder : public AutoSerializable {
  public:
    
    //! \brief NullPlaceholder constructor
    NullPlaceholder();

  AUTO_SER_STUFF_WITHOUT_MEMBERS
};
#undef className

#endif
