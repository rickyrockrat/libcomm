//! \file exception.h
//! \brief Exception class
//! \author Alexandre Roux
//! \version 0.1
//! \date 13 mars 2009
//!
//! File containing the declarations of the class Exception.
#ifndef EXCEPTION_H
#define EXCEPTION_H

#define EX_STREAM_TIMEOUT -1
#define EX_ACCEPT_TIMEOUT -2
#define EX_STREAM_CLOSED -3
#define EX_EOF -4
#define EX_OSTREAM_TOO_MUCH_DATA -5
#define EX_ISTREAM_VIRTUAL_CALL -6
#define EX_CONDITION_TIMEOUT ETIMEDOUT

#include <string>

//! \class Exception libcomm/exception.h
//! \brief Exception class
class Exception {

  private :
    int code;
    std::string message;

  public :

    //! \brief Exception constuctor
    //!
    //! Creates a new Exception.
    Exception(void);

    //! \brief Exception constuctor
    //!
    //! Creates a new Exception.
    Exception(int code);

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
