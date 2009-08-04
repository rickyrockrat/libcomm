#ifndef NET_SOCKET_H
#define NET_SOCKET_H

#include <netinet/in.h>  // sockaddr_in
#include <string>

#include "serializable.h"
#include "net_address.h"
#include "exception.h"

#define SOCKET_CLOSED -3
#define SOCKET_TIMEOUT -4
#define SOCKET_CONNECTION_FAILURE -5
#define SOCKET_ID_WRONG -6
#define NB_NSEC_IN_SEC 1000000000

class NetSocket {
  protected :
    int socketId;
    
    NetSocket();
    NetSocket(int domain);
    virtual ~NetSocket();
    static void launchNetExceptionForSocket(int code);
    static void launchNetExceptionForSelect(int code);
    static void launchNetExceptionForSendReceive(int code);
    static void launchNetExceptionForIoctl(int code);
    static void launchNetExceptionForGetSockName(int code);
    static void launchNetExceptionForGetSetSockOpt(int code);
    void bindSocket(int port);
    void computeTimeDiff(time_t secf, long nanosecf, time_t secs, long nanosecs,
      time_t &secr, long nanosecr);

    friend class NetAddress;

  public :
    void closeSocket();
    NetAddress getLocalAddress() const;
    NetAddress getLocalAddress(const char *interface) const;
    std::map<std::string, NetAddress> *getLocalAddresses() const;
    
    class NetException : public Exception {
       
      public :
        NetException(int code, std::string message);
    };

    class Option {
      public :
        Option(int name, socklen_t len);
        virtual ~Option(void);
      protected :
        int name;
        socklen_t len;

        virtual void *getValue(void) = 0;
        virtual socklen_t *getSize(void);
        int getName(void);

      friend class NetSocket;
    };

    class IntOption : public Option {
      public:
        virtual ~IntOption(void);
      protected :
        int value; 
        
        IntOption(int name, int value);
        void *getValue(void);
    };

    class IntegerOption : public IntOption {
      public :
        enum Name {
          sndBufOpt,
          rcvBufOpt,
          sndLowAtOpt,
          rcvLowAtOpt
        };
        int getIntegerValue();
        IntegerOption(IntegerOption::Name name, int value);
        IntegerOption(IntegerOption::Name name);
        ~IntegerOption(void);
      private:
       static const int names[]; 
    };

   class BooleanOption : public IntOption {
      public :
        enum Name {
          debugOpt,
          broadcastOpt,
          reuseAddrOpt,
          keepAliveOpt,
          oobInlineOpt,
          dontRouteOpt
        };
        bool getBooleanValue();
        BooleanOption(BooleanOption::Name name, bool value);
        BooleanOption(BooleanOption::Name name);
        ~BooleanOption(void);
      private:
       static const int names[]; 
    };

    void setSocketOption(NetSocket::Option &option);
    void getSocketOption(NetSocket::Option *option);

};


#endif
