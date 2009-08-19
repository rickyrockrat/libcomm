//! \file net_address.h
//! \brief Network address container
//! \author Alexandre Roux
//! \version 0.1
//! \date 13 mars 2009
//!
//! File containing the declaration of a network address container.

#ifndef NET_ADDRESS_H
#define NET_ADDRESS_H

#include <map>
#include <arpa/inet.h>

#include "exception.h"

//! \class NetAddress libcomm/structs/net_address.h
//! \brief Network address container
//!
//! This class is a container of a network end point. It provides methods to
//! resolve ip from name and name from ip.
class NetAddress {
  
  private :
    bool resolved;
    std::string ip;
    std::string hostname;
    std::string address;
    int port;

    static void launchNetExceptionForNameResolution(int code);

    friend class NetSocket;

  public :
    
    //! \brief NetAddress constructor
    //!
    //! Default constructor. Address is undifined.
    NetAddress(void);

    //! \brief NetAddress constructor
    //! \param[in] address the address
    //! \param[in] port the port
    //!
    //! Creates a new NetAddress and set its address and port to the given
    //! parameters values. The address parameter can be either an ip
    //! in string representation or a canonical name. Note that it is the value
    //! returned by the getAddress() function.
    NetAddress(std::string address, int port);
    
    //! \brief NetAddress constructor
    //! \param[in] addr the address structure
    //!
    //! Creates a new NetAddress from a sockaddr_in structure. This constructor
    //! is mainly used by the NetSocket class and its subclasses.
    NetAddress(const sockaddr_in &addr);


    //! \brief Gets address string
    //! \return the address
    //!
    //! Returns a copy of the address associated with this NetAddress. Note that
    //! this field can be a string representation of an ip or a canonical name.
    std::string getAddress(void) const;

    //! \brief Gets address port
    //! \return the port
    //!
    //! Returns the port associated with this NetAddress.
    int getPort(void) const;

    //! \brief Sets address string
    //! \param[in] the address
    //!
    //! Sets the address associated with this NetAddress.
    void setAddress(std::string address);
    
    //! \brief Sets address port
    //! \param[in] the port
    //!
    //! Sets the port associated with this NetAddress.
    void setPort(int port);
    
    //! \brief Gets address canonical name
    //! \return the canonical name
    //!
    //! Returns the address canonical name associated with this NetAddress.
    //! Note that, if not already done, this will call resolve().
    std::string getCanonicalName(void);
    
    //! \brief Gets address Ip
    //! \return a string representation of the ip
    //!
    //! Returns a string representation of the ip associated with this
    //! NetAddress. Note that, if not already done, this will call resolve().
    std::string getIp(void);

    //! \brief Converts this NetAddress into sockaddr_in
    //! \param[out] saddr the sockaddr_in pointer to copy to
    //!
    //! Converts this NetAddress into a sockaddr_in structure.
    void getSockAddr(sockaddr_in *saddr) const;


    //! \brief Resolves address
    //!
    //! Resolve the address and find ip and canonical name.
    void resolve(void);
    static std::string getLocalHostname(void);
    static std::string getLocalIp(void);
    static std::map<std::string,NetAddress> *getInterfaces(void);
    static NetAddress getInterface(const char *interface);
    static std::string findIp(std::string address);
    static std::string findHostname(std::string address);
    static std::string findHostname(const sockaddr_in &addr);

    class NetException : public Exception {
      public :
        NetException(int code, std::string message);
    };
};

#endif
