#include "logger.h"

#include <time.h>
#include <typeinfo>

const char * typeDisplayingString[NB_DISPLAY_MESSAGE_TYPE] = { 
    " Finest   :",
    " Finer    :",
    " Fine     :",
    " Debug    :",
    " Info     :",
    " PerfTest :",
    " Warning  :",
    " Error    :"
    };


LoggerListener::LoggerListener(): messagePolicy(INFO) {}

LoggerListener::~LoggerListener() {}

void LoggerListener::setDisplayMessageLevel(typeDisplayingMessages t) {
  messagePolicy = t;
}

void LoggerListener::open() {}

void LoggerListener::close() {}

LoggerConsoleListener::LoggerConsoleListener() 
  : LoggerListener() {
}

LoggerConsoleListener::~LoggerConsoleListener() {}

void LoggerConsoleListener::notify(LogMessage &message) {
  if (message.getType() >= messagePolicy) {
    std::cout << message << std::endl;
  }
}

LoggerFileListener::LoggerFileListener(const char *filen):
  filename(filen) {
}

LoggerFileListener::~LoggerFileListener() {}

void LoggerFileListener::open() {
  file = new std::ofstream(filename);
}

void LoggerFileListener::notify(LogMessage &message) {
  if ((*file) && (message.getType() >= messagePolicy)) {
    *file << message << std::endl;
  }
}

void LoggerFileListener::close() {
  file->close();
  delete file;
}

LoggerStoreListener::LoggerStoreListener() 
  : LoggerListener() {
}

LoggerStoreListener::~LoggerStoreListener() {}

void LoggerStoreListener::notify(LogMessage &message) {
  messages.push_back(message);
}

std::vector<LogMessage> &LoggerStoreListener::getMessages() {
  return messages; 
}

LogMessage::LogMessage( typeDisplayingMessages typeMessage,
                        std::string className,
                        std::string message) {
  this->typeMessage = typeMessage;
  this->className = className;
  this->timeMessage = time(NULL);
  this->message = message;
}

std::string LogMessage::getMessage() {
  return message;
}

typeDisplayingMessages LogMessage::getType() {
  return typeMessage;
}

time_t LogMessage::getTime() {
  return timeMessage;
}

std::string LogMessage::getClassName() {
  return className;
}

std::ostream &operator<<(std::ostream &a, LogMessage &message) {
  char * current_time = asctime(localtime(&message.timeMessage));
  current_time[24] = '\0';
  std::string className = " ";
  if (message.className != "") {
    className = " (" + message.className + ") ";
  }
  a << "[" << current_time << "]" << typeDisplayingString[message.typeMessage] 
    << className << message.message;
  return a;
}

int operator<(LogMessage m1, LogMessage m2) {
  double diff = difftime(m1.getTime(),m2.getTime());
  return (diff > 0);
}

Logger Logger::self;
bool Logger::initied = false;
Mutex Logger::lock(Mutex::recursiveType);
Mutex Logger::lockForLock;
int Logger::mutexLockCount = 0;
bool Logger::threadSafe = true;
std::map<std::string, LoggerListener*> Logger::listeners;

void Logger::init() {

  std::map<std::string, LoggerListener*>::iterator iter;

  for(iter = listeners.begin(); iter != listeners.end(); ++iter) {
    iter->second->open();
  }

  initied = true;
}

void Logger::cleanup() {
  std::map<std::string, LoggerListener*>::iterator iter;

  for(iter = listeners.begin(); iter != listeners.end(); ++iter) {
    iter->second->close();
    delete iter->second;
  }
  listeners.clear();
}

void Logger::addListener(std::string name, LoggerListener *listener) {
  listeners[name] = listener;
  if (initied) {
    listener->open();
  }
}

LoggerListener *Logger::getListener(std::string name) {
  std::map<std::string, LoggerListener*>::iterator find;

  find = listeners.find(name);
  if (find == listeners.end()) {
    return NULL;
  } else {
    return find->second;
  }

}

void Logger::removeListener(std::string name) {
  listeners.erase(name);
}

std::stringstream &Logger::log(typeDisplayingMessages t) {
  if (threadSafe) {
    lock.lock();
    lockForLock.lock();
    lockForLock.unlock();
    mutexLockCount++;
  }
  Logger::self.currentDisplayingMessages = t;
  return Logger::self.messageStream;
}

std::stringstream &Logger::log() {
  if (threadSafe) {
    lock.lock();
    lockForLock.lock();
    lockForLock.unlock();
    mutexLockCount++;
  }
  return Logger::self.messageStream;
}

Logger::Logger() {}

LoggerEndMessageWithoutName *Logger::endm() {
  return new LoggerEndMessageWithoutName();
}

Logger::CustomName *Logger::endmwn(std::string name) {
  return new Logger::CustomName(name);
}

void Logger::setThreadSafe(bool threadSafe) {
  Logger::threadSafe = threadSafe;
}

void Logger::flushLastMessage(std::string className) {
  std::map<std::string, LoggerListener*>::iterator iter;
  LogMessage m(Logger::self.currentDisplayingMessages, className,
    Logger::self.messageStream.str());

  for(iter = listeners.begin(); iter != listeners.end(); ++iter) {
    iter->second->notify(m);
  }
  if (threadSafe) {
    lockForLock.lock();
    Logger::self.messageStream.str("");
    for (;0<mutexLockCount; --mutexLockCount) {
      lock.unlock();
    }
    lockForLock.unlock();
  } else {
    Logger::self.messageStream.str("");
  }

}

void LoggerEndMessageWithoutName::flush() {
  Logger::flushLastMessage(""); 
}

std::ostream &operator<<(std::ostream &a, LoggerEndMessageWithoutName *lph) {
  lph->flush();
  delete lph;
  return a;
}

void Logger::CustomName::flush() {
  Logger::flushLastMessage(name);
}

std::ostream &operator<<(std::ostream &a, Logger::CustomName *cn) {
  cn->flush();
  delete cn;
  return a;
}
