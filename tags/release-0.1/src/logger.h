#ifndef LOGGER_H
#define LOGGER_H
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <typeinfo>

#include "mutex.h"

//TODO Add support for logmessage browsing

enum typeDisplayingMessages {
    FINEST,
    FINER,
    FINE,
    DEBUG,
    INFO,
    PERFORMANCE_TEST,
    WARNING,
    ERROR,
    NB_DISPLAY_MESSAGE_TYPE
};

class LogMessage {
  public :
    LogMessage( 
      typeDisplayingMessages typeMessage,
      std::string className,
      std::string message);
    std::string getMessage();
    typeDisplayingMessages getType();
    time_t getTime();
    std::string getClassName();

    friend std::ostream &operator<<(std::ostream &a, LogMessage &message);

  private :
    time_t timeMessage;
    std::string className;
    std::string message;
    typeDisplayingMessages typeMessage;
};

class LoggerListener {
  public :
    void setDisplayMessageLevel(typeDisplayingMessages t);

  protected:
    typeDisplayingMessages messagePolicy;

    LoggerListener();
    virtual ~LoggerListener();
    virtual void open();
    virtual void notify(LogMessage &message) = 0;
    virtual void close();

  friend class Logger;
};

class LoggerConsoleListener : public LoggerListener {
  public:
    LoggerConsoleListener();
    virtual ~LoggerConsoleListener();

  private :
    void notify(LogMessage &message);
};

class LoggerFileListener : public LoggerListener {
  public:
    LoggerFileListener(const char *filen);
    virtual ~LoggerFileListener();

  private:
    const char *filename;
    std::ofstream *file;

    void open();
    void notify(LogMessage &message);
    void close();
};

class LoggerStoreListener : public LoggerListener {
  public:
    LoggerStoreListener();
    virtual ~LoggerStoreListener();
    std::vector<LogMessage> &getMessages();

  private :
    std::vector<LogMessage> messages; 
    void notify(LogMessage &message);
};

template <typename T>
class LoggerEndMessage {
  public :
    void flush();
  private :
    LoggerEndMessage<T>() {}
    
    std::string getClassName() {
      std::string str(typeid(T).name());
#ifdef __GNUC__
      int i = 0;
      while ((('A' <= str[i]) && (str[i] <= 'Z')) ||
            (('a' <= str[i]) && (str[i] <= 'z'))) {
        ++i;
      }
      while (('0' <= str[i]) && (str[i] <= '9')) {
        ++i;
      }
      return str.substr(i,str.length());
#else
      return str;
#endif
    }

    friend class Logger;
};

template <typename T>
std::ostream &operator<<(std::ostream &a, LoggerEndMessage<T> *lph) {
  lph->flush();
  delete lph;
  return a;
}

class LoggerEndMessageWithoutName {
  public :
    void flush();
};

std::ostream &operator<<(std::ostream &a, LoggerEndMessageWithoutName *lph);

int operator<(LogMessage m1, LogMessage m2);

class Logger {
  public:
    
    static void init();
    static void cleanup();
    static void addListener(std::string name, LoggerListener *listener);
    static LoggerListener *getListener(std::string name);
    static void removeListener(std::string name);
    static std::stringstream &log(typeDisplayingMessages t);
    static std::stringstream &log();
    static void setThreadSafe(bool threadSafe);
    static void flushLastMessage(std::string className);
    
    template<typename T>
    static void flushLastMessage(LoggerEndMessage<T> &lem) {
      flushLastMessage(lem.getClassName());
    }
    
    template<typename T>
    static LoggerEndMessage<T> *endm(T cn) {
      return new LoggerEndMessage<T>();
    }
    static LoggerEndMessageWithoutName *endm();

    class CustomName {
      public :
        CustomName(std::string &name) {
          this->name = name;
        }
        void flush();
      private :
        std::string name;

    };

    static Logger::CustomName *endmwn(std::string name);
    
    
  private :
    static Logger self;
    static bool initied;
    typeDisplayingMessages currentDisplayingMessages;
    std::stringstream messageStream;
    static std::map<std::string, LoggerListener*> listeners;
    static bool threadSafe;
    static Mutex lock;
    static int mutexLockCount;
    static Mutex lockForLock;

    Logger();
    void insert(LogMessage &m);
};


std::ostream &operator<<(std::ostream &a, Logger::CustomName *cn);

template <typename T>
void LoggerEndMessage<T>::flush() {
  Logger::flushLastMessage(*this);
}

#endif
