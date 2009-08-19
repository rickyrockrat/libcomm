#ifndef NET_SOCKET_H
#define NET_SOCKET_H

#include <netinet/in.h>  // sockaddr_in
#include <string>

#include "serializable.h"
#include "net_address.h"
#include "exception.h"
#include "input_stream.h"
#include "output_stream.h"

class NetSocket {
  protected :
    
    NetSocket(void);
    virtual ~NetSocket();
    
    int createSocket(int domain);
    void bindSocket(int port);

    virtual int getSocketId(void) const = 0;

    friend class NetAddress;
  public :
    
    NetAddress getLocalAddress() const;
    NetAddress getLocalAddress(const char *interface) const;
    std::map<std::string, NetAddress> *getLocalAddresses() const;
    
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
          rcvLowAtOpt,
          error
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

    class NetException : public Exception {
      public :
        NetException(int code);
        NetException(int code, std::string message);
    };
};

class IONetSocket: public NetSocket, public BufferedInputStream, public OutputStream {
  protected:
    int getSocketId(void) const;

  public:
    IONetSocket(void);
    IONetSocket(int domain);
    IONetSocket(int domain, const NetAddress &address);

    void connectSocket(const NetAddress &address);
    void connectSocket(const NetAddress &address, uint64_t nanosec);
    void connectSocket(const NetAddress &address, time_t sec, long nanosec);

    NetAddress getDistantAddress() const;
};

#endif
