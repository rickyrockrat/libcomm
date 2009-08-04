//! \file exception.h
//! \brief Exception class
//! \author Alexandre Roux
//! \version 0.1
//! \date 13 mars 2009
//!
//! File containing the declarations of the class Exception.
#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <string>

//! \class Exception libcomm/exception.h
//! \brief Exception class
class Exception {

  private :
    int code;
    std::string message;

  public :

    //! \brief Exception constuctor
    //! \param code the exception code
    //! \param message the exception message
    //!
    //! Creates a new Exception with the given code and message.
    Exception(int code, std::string message);

    //! \brief Gets the code
    //! \return the Exception code
    //!
    //! Returns the Exception code
    int getCode();

    //! \brief Gets the message
    //! \return the Exception message
    //!
    //! Returns the Exception message
    std::string getMessage();

    //! \brief Prints the code and Message
    void printCodeAndMessage();
};

#endif
