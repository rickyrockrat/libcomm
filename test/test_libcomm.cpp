#include <iostream>
#include <time.h>
#include <sstream>
#include <stdlib.h>

#include "libcomm/libcomm.h"
#include "libcomm/libcomm_structs.h"
#include "libcomm/net_address.h"
#include "libcomm/udp_socket.h"
#include "libcomm/tcp_socket.h"
#include "libcomm/thread.h"
#include "libcomm/mutex.h"
#include "libcomm/timer.h"
#include "libcomm/logger.h"
#include "libcomm/config_loader.h"

#include "test_libcomm_testautoser.h"

#define PORT 5555
#define ADDRESS NetAddress::getLocalIp()
#define NUMBER_TEST 28

typedef bool (*CompareFunc) (const void* first, const void* second);
bool compareString(const void* first, const void* second);
bool compareStringPtr(const void* first, const void* second);
bool compareVectorChar(const void* first, const void* second);
bool compareVectorFloat(const void* first, const void* second);
bool compareVectorDouble(const void* first, const void* second);
bool compareVectorInt8(const void* first, const void* second);
bool compareVectorInt16(const void* first, const void* second);
bool compareVectorInt32(const void* first, const void* second);
bool compareVectorInt64(const void* first, const void* second);
bool compareVectorUInt8(const void* first, const void* second);
bool compareVectorUInt16(const void* first, const void* second);
bool compareVectorUInt32(const void* first, const void* second);
bool compareVectorUInt64(const void* first, const void* second);
bool compareVectorString(const void* first, const void* second);
bool compareVectorStringPtr(const void* first, const void* second);
bool compareBufferChars(const void* first, const void* second);
bool compareBufferUint64t(const void* first, const void* second);
bool compareMapUin64tBuffer(const void* first, const void* second);
bool compareMapCharString(const void* first, const void* second);
bool compareMapCharStringPtr(const void* first, const void* second);
bool compareMapStringPtrChar(const void* first, const void* second);
bool compareMapStringPtrStringPtr(const void* first, const void* second);
bool compareTestSerClass(const void* first, const void* second);
bool compareTestSimpleSerializable(const void* first, const void* second);
bool compareTestAutoSerializable(const void* first, const void* second);
bool compareTestSubAutoSerializable(const void* first, const void* second);
bool compareTestSubSubAutoSerializable(const void* first, const void* second);

class ReceiverThread : public Thread {
  
  private :
    Serializable** receivedData;
    Serializable** sentData;
    CompareFunc* comparFuncs;
    std::string *testNames;
    bool tcp;
    int port;

  public :
    ReceiverThread( Serializable** receivedData,
                    Serializable** sentData,
                    CompareFunc* comparFuncs,
                    std::string *testNames,
                    bool tcp);
    ReceiverThread( Serializable** receivedData,
                    Serializable** sentData,
                    CompareFunc* comparFuncs,
                    std::string *testNames,
                    bool tcp,
                    int port);
    virtual ~ReceiverThread();
    void *run();
};

class SenderThread : public Thread {
  
  private :
    Serializable **sentData;
    NetAddress *address;
    bool tcp;

  public :
    SenderThread(Serializable** sentData, bool tcp);
    SenderThread(Serializable** sentData, NetAddress *address, bool tcp);
    virtual ~SenderThread();
    void *run();
};


//little class for testing
class TestSerClass : public Serializable {
  private :
    String *str;
    Vector<int> *vect;
    Vector<Vector<String> > *doubleVect;
    
    //Methods and fields needed for serialization
    static uint16_t type;

    NetMessage *serialize() const;
    virtual uint16_t getType() const;
    static Serializable *deserialize(const NetMessage &data, bool ptr);

    friend class libcomm;

  public :
    TestSerClass(bool init);
    virtual ~TestSerClass();
    
    bool operator==(TestSerClass &c);
};


uint16_t TestSerClass::type = 10000;

TestSerClass::TestSerClass(bool init) {
  if (init) {
    //Hard init of field
    str = new String("I am a string in TestSerClass!");
    vect = new Vector<int>();
    vect->push_back(2173);
    vect->push_back(42);
    vect->push_back(-12345);
    doubleVect = new Vector<Vector<String> >();
    Vector<String> vect1;
    vect1.push_back("String 1 in vect 1");
    vect1.push_back("String 2 in vect 1");
    vect1.push_back("String 3 in vect 1");
    Vector<String> vect2;
    vect2.push_back("String 1 in vect 2");
    vect2.push_back("String 2 in vect 2");
    vect2.push_back("String 3 in vect 2");
    Vector<String> vect3;
    vect3.push_back("String 1 in vect 3");
    vect3.push_back("String 2 in vect 3");
    vect3.push_back("String 3 in vect 3");
    doubleVect->push_back(vect1);
    doubleVect->push_back(vect2);
    doubleVect->push_back(vect3);
  }
}

TestSerClass::~TestSerClass() {
  delete str;
  delete vect;
  delete doubleVect;
}

NetMessage *TestSerClass::serialize() const {
  SerializationManager *sm = SerializationManager::getSerializationManager();
  NetMessage *message = new NetMessage(this->getType(),(char*) NULL,0);
  message = sm->serialize(*str, message);
  message = sm->serialize(*vect,message);
  message = sm->serialize(*doubleVect,message);
  return message;
}

Serializable *TestSerClass::deserialize(const NetMessage &data, bool ptr) {
  SerializationManager *sm = SerializationManager::getSerializationManager();
  TestSerClass *tsc = new TestSerClass(false);
  const std::vector<NetMessage*> &messages = data.getMessages();

  int currentPos = -1;
  String *s = (String*) sm->deserialize(messages[++currentPos],false);
  tsc->str = s;
  Vector<int> *v = (Vector<int>*) sm->deserialize(messages[++currentPos], false);
  tsc->vect = v;
  Vector<Vector<String> > *db = (Vector<Vector<String> >*) sm->deserialize(messages[++currentPos], false);
  tsc->doubleVect = db;

  return tsc;
}

uint16_t TestSerClass::getType() const {
  return type;
}


bool TestSerClass::operator==(TestSerClass &c) {
  if (*str != *(c.str)) {
    return false;
  }
  if (vect->size() != c.vect->size()) {
    return false;
  }
  for (unsigned i(0); i<vect->size(); ++i) {
    if ((*vect)[i] != (*(c.vect))[i]) {
    return false;
    }
  }
  if (doubleVect->size() != doubleVect->size()) {
    return false;
  }
  for (unsigned i(0); i<doubleVect->size(); ++i) {
    if ((*doubleVect)[i].size() != (*(c.doubleVect))[i].size()) {
      return false;
    }
    for (unsigned j(0); j<(*doubleVect)[i].size(); ++j) {
      if ((*doubleVect)[i][j] != (*(c.doubleVect))[i][j]) {
        return false;
      }
    }
  }
  return true;
}

#define className TestSimpleSerializable
class TestSimpleSerializable : public SimpleSerializable {
  public :
    TestSimpleSerializable(uint16_t a, uint32_t b, uint64_t c,
      char d, float e, double f) {
      SIMPLE_SER_INIT;
      this->a = a;
      this->b = b;
      this->c = c;
      this->d = d;
      this->e = e;
      this->f = f;
    }
    bool operator==(TestSimpleSerializable &tss);
  private :
    uint16_t a;
    uint32_t b;
    uint64_t c;
    char d;
    float e;
    double f;
    TestSimpleSerializable() {}
    SIMPLE_SER_STUFF
};
SIMPLE_SER_STATIC_STUFF
#undef className


bool TestSimpleSerializable::operator==(TestSimpleSerializable &tss) {
  return ((a == tss.a) && (b == tss.b) && (c == tss.c)
    && (d == tss.d) && (e = tss.e) && (f == tss.f));
}

#define className TestSubAutoSerializable
class TestSubAutoSerializable : public TestAutoSerializable {
  public :
    TestSubAutoSerializable( uint64_t a, double *b, String str, 
                          String *strPtr, Vector<char> *v, int64_t i)
      : TestAutoSerializable(a,b,str,strPtr,v),i(i) {}
    ~TestSubAutoSerializable() {}
    TestSubAutoSerializable(){} 
    bool operator==(TestSubAutoSerializable &tas);
  protected :
    virtual void copyFields (const TestSubAutoSerializable &t) {
      TestAutoSerializable::copyFields(t);
      seti(t.geti());
    }

    
  AUTO_SER_PRIMITIVE_FIELD_WITH_GET_AND_SET(int64_t, i)

  AUTO_SER_STUFF_WITH_SUPERCLASS(TestAutoSerializable,ITEM(i))
};
AUTO_SER_STATIC_STUFF
#undef className

bool TestSubAutoSerializable::operator==(TestSubAutoSerializable &tas) {
  return ((*(TestAutoSerializable*)(this) == (TestAutoSerializable&)tas) && (i = tas.i));
}

#define className TestSubSubAutoSerializable
class TestSubSubAutoSerializable : public TestSubAutoSerializable {
  public :
    TestSubSubAutoSerializable( uint64_t a, double *b, String str, 
                          String *strPtr, Vector<char> *v, int64_t i,
                          float f)
      : TestSubAutoSerializable(a,b,str,strPtr,v,i),f(f) {}
    ~TestSubSubAutoSerializable() {}
    TestSubSubAutoSerializable() {} 
    bool operator==(TestSubSubAutoSerializable &tas);
    
  AUTO_SER_PRIMITIVE_FIELD_WITH_GET_AND_SET(float, f)

  AUTO_SER_STUFF_WITH_SUPERCLASS(TestSubAutoSerializable,ITEM(f))
};
AUTO_SER_STATIC_STUFF
#undef className

bool TestSubSubAutoSerializable::operator==(TestSubSubAutoSerializable &tas) {
  return ((*(TestSubAutoSerializable*)(this) == (TestSubAutoSerializable&)tas) && (f = tas.f));
}

void printTest(std::string name, bool result) {
  Logger::log(DEBUG) << name ;
  for (int i = name.length(); i< 70; ++i) {
     Logger::log() << ".";
  }
  Logger::log() <<   (result 
                  ? "[  \033[1;32mOk(B[m  ]" 
                  : "[\033[1;31mFAILED(B[m]")
  << Logger::endmwn("Main");
}

void fillTestData(  Serializable **sentData,
                    CompareFunc *comparFuncs,
                    std::string *testNames,
                    bool tcp) {
  int i = -1;
  
  if (tcp) {
    sentData[++i] = new String (""
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
      "adlkaélskfdéakfaskdfélaskfélsakdfélsakdlékflsféksféasdéfkfkéadkfélk"
    );
  } else {
    sentData[++i] = new String ("String test! ·]#¼]}½]¬}¢|}¼#adpèsjiofkélvnyv");
  }

  comparFuncs[i] = &compareString;
  testNames[i] = std::string("String");

  Vector<char> *vCh = new Vector<char>();
  vCh->push_back('a');
  vCh->push_back('b');
  vCh->push_back('c');
  sentData[++i] = vCh;
  comparFuncs[i] = &compareVectorChar;
  testNames[i] = std::string("Vector<char>");

  
  Vector<float> *vFl = new Vector<float>();
  vFl->push_back(1.2f);
  vFl->push_back(3.5f);
  vFl->push_back(1234.45f);
  sentData[++i] = vFl;
  comparFuncs[i] = &compareVectorFloat;
  testNames[i] = std::string("Vector<float>");
  
  Vector<double> *vD = new Vector<double>();
  vD->push_back(18904.1928490);
  vD->push_back(21384.129034);
  vD->push_back(4294967296ULL);
  sentData[++i] = vD;
  comparFuncs[i] = &compareVectorDouble;
  testNames[i] = std::string("Vector<double>");
  
  Vector<int8_t> *vI8 = new Vector<int8_t>();
  vI8->push_back(9);
  vI8->push_back(124);
  vI8->push_back(-127);
  sentData[++i] = vI8;
  comparFuncs[i] = &compareVectorInt8;
  testNames[i] = std::string("Vector<int8_t>");

  Vector<int16_t> *vI16 = new Vector<int16_t>();
  vI16->push_back(78);
  vI16->push_back(32700);
  vI16->push_back(-31492);
  sentData[++i] = vI16;
  comparFuncs[i] = &compareVectorInt16;
  testNames[i] = std::string("Vector<int16_t>");

  Vector<int32_t> *vI32 = new Vector<int32_t>();
  vI32->push_back(129);
  vI32->push_back(2147483646);
  vI32->push_back(-2147483640);
  sentData[++i] = vI32;
  comparFuncs[i] = &compareVectorInt32;
  testNames[i] = std::string("Vector<int32_t>");

  Vector<int64_t> *vI64 = new Vector<int64_t>();
  vI64->push_back(87);
  vI64->push_back(9223372036854775806LL);
  vI64->push_back(-9223372036854775805LL);
  sentData[++i] = vI64;
  comparFuncs[i] = &compareVectorInt64;
  testNames[i] = std::string("Vector<int64_t>");
  
  Vector<uint8_t> *vUI8 = new Vector<uint8_t>();
  vUI8->push_back(9);
  vUI8->push_back(124);
  vUI8->push_back(255);
  sentData[++i] = vUI8;
  comparFuncs[i] = &compareVectorUInt8;
  testNames[i] = std::string("Vector<uint8_t>");

  Vector<uint16_t> *vUI16 = new Vector<uint16_t>();
  vUI16->push_back(78);
  vUI16->push_back(32700);
  vUI16->push_back((uint16_t) 65634);
  sentData[++i] = vUI16;
  comparFuncs[i] = &compareVectorUInt16;
  testNames[i] = std::string("Vector<uint16_t>");

  Vector<uint32_t> *vUI32 = new Vector<uint32_t>();
  vUI32->push_back(129);
  vUI32->push_back(2147483646);
  vUI32->push_back(4294967295UL);
  sentData[++i] = vUI32;
  comparFuncs[i] = &compareVectorUInt32;
  testNames[i] = std::string("Vector<uint32_t>");

  Vector<uint64_t> *vUI64 = new Vector<uint64_t>();
  vUI64->push_back(87);
  vUI64->push_back(4611686018427387903ULL);
  vUI64->push_back(18446744073709551615ULL);
  sentData[++i] = vUI64;
  comparFuncs[i] = &compareVectorUInt64;
  testNames[i] = std::string("Vector<uint64_t>");

  Vector<String> *vStr = new Vector<String>();
  vStr->push_back("String 1 àé$éé$");
  vStr->push_back("String 2 }´[}{");
  vStr->push_back("String 3 @#¼¼½¬#½¼");
  vStr->push_back("String 4 ;:_+\"*ç%&/");
  sentData[++i] = vStr;
  comparFuncs[i] = &compareVectorString;
  testNames[i] = std::string("Vector<String>");

  Vector<String*> *vStrPtr = new Vector<String*>();
  vStrPtr->push_back(new String("String 1 àé$éé$"));
  vStrPtr->push_back(new String("String 2 }´[}{"));
  vStrPtr->push_back(new String("String 3 @#¼¼½¬#½¼"));
  vStrPtr->push_back(new String("String 4 ;:_+\"*ç%&/"));
  sentData[++i] = vStrPtr;
  comparFuncs[i] = &compareVectorStringPtr;
  testNames[i] = std::string("Vector<String*>");
  
  Vector<char> *vECh = new Vector<char>();
  sentData[++i] = vECh;
  comparFuncs[i] = &compareVectorChar;
  testNames[i] = std::string("Vector<char> (empty)");

  Buffer<char> *bCh = new Buffer<char>();
  const char *test_bch;
  if (tcp) {
    test_bch =  "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!"
                "Test buffer<char>!Test buffer<char>!Test buffer<char>!";
  } else {
    test_bch = "Test buffer<char>!";
  }
  bCh->copyIn(0,test_bch,strlen(test_bch));
  sentData[++i] = bCh;
  comparFuncs[i] = &compareBufferChars;
  testNames[i] = std::string("Buffer<char>");

  Buffer<uint64_t> *bUi = new Buffer<uint64_t>();
  int max;
  if (tcp) max = 4096;
  else max = 128;
  for (int i = 0; i<max; ++i) {
    (*bUi)[i] = i;
  }
  sentData[++i] = bUi;
  comparFuncs[i] = &compareBufferUint64t;
  testNames[i] = std::string("Buffer<uint64_t>");

  Map<uint64_t, Buffer<char>*> *mapBuf = new Map<uint64_t, Buffer<char>*>();
  Buffer<char> *buffMap = new Buffer<char>();
  buffMap->copyIn(0,test_bch,strlen(test_bch));
  (*mapBuf)[0x0123] = buffMap;
  sentData[++i] = mapBuf;
  comparFuncs[i] = &compareMapUin64tBuffer;
  testNames[i] = std::string("Map<uint64_t, Buffer<char> >");


  Map<char,String> *mapChStr = new Map<char,String>();
  (*mapChStr)['a'] = "String 1 àé$éé$";
  (*mapChStr)['b'] = "String 2 }´[}{";
  (*mapChStr)['c'] = "String 3 @#¼¼½¬#½¼";
  (*mapChStr)['d'] = "String 4 ;:_+\"*ç%&/";
  sentData[++i] = mapChStr;
  comparFuncs[i] = &compareMapCharString;
  testNames[i] = std::string("Map<char,String>");

  Map<char,String*> *mapChStrPtr = new Map<char,String*>();
  (*mapChStrPtr)['a'] = new String("String 1 àé$éé$");
  (*mapChStrPtr)['b'] = new String("String 2 }´[}{");
  (*mapChStrPtr)['c'] = new String("String 3 @#¼¼½¬#½¼");
  (*mapChStrPtr)['d'] = new String("String 4 ;:_+\"*ç%&/");
  sentData[++i] = mapChStrPtr;
  comparFuncs[i] = &compareMapCharStringPtr;
  testNames[i] = std::string("Map<char,String*>");

  Map<String*,char,strPtrCmp> *mapStrPtrCh = new Map<String*,char,strPtrCmp>();
  (*mapStrPtrCh)[new String("String 1 àé$éé$")] = 'a';
  (*mapStrPtrCh)[new String("String 2 }´[}{")] = 'b';
  (*mapStrPtrCh)[new String("String 3 @#¼¼½¬#½¼")] = 'c';
  (*mapStrPtrCh)[new String("String 4 ;:_+\"*ç%&/")] = 'd';
  sentData[++i] = mapStrPtrCh;
  comparFuncs[i] = &compareMapStringPtrChar;
  testNames[i] = std::string("Map<String*,char,strPtrCmp>");
  
  Map<String*,String*,strPtrCmp> *mapStrPtrStrPtr = new Map<String*,String*,strPtrCmp>();
  (*mapStrPtrStrPtr)[new String("String 1 àé$éé$")] = new String("String 1 value");
  (*mapStrPtrStrPtr)[new String("String 2 }´[}{")] = new String("String 2 value");
  (*mapStrPtrStrPtr)[new String("String 3 @#¼¼½¬#½¼")] = new String("String 3 value");
  (*mapStrPtrStrPtr)[new String("String 4 ;:_+\"*ç%&/")] = new String("String 4 value");
  sentData[++i] = mapStrPtrStrPtr;
  comparFuncs[i] = &compareMapStringPtrStringPtr;
  testNames[i] = std::string("Map<String*,String*,strPtrCmp>");
  
  Map<char,String> *mapEChStr = new Map<char,String>();
  sentData[++i] = mapEChStr;
  comparFuncs[i] = &compareMapCharString;
  testNames[i] = std::string("Map<char,String> (empty)");

  TestSerClass *tsc = new TestSerClass(true);
  sentData[++i] = tsc;
  comparFuncs[i] = &compareTestSerClass;
  testNames[i] = std::string("TestSerClass");
  
  TestSimpleSerializable *tss = new TestSimpleSerializable(1,2,3,'c',1.2,3455.6777);
  sentData[++i] = tss;
  comparFuncs[i] = &compareTestSimpleSerializable;
  testNames[i] = std::string("TestSimpleSerializable");
  
  String *sas = new String("TestAutoSerial");
  Vector<char> *vectas = new Vector<char>();
  vectas->push_back('a');
  vectas->push_back('b');
  vectas->push_back('c');
  vectas->push_back('d');
  double *d = new double;
  *d = 87.234;
  sentData[++i] = new TestAutoSerializable(18446744073709551615ULL,d,"asdfsadf",sas,vectas);
  comparFuncs[i] = &compareTestAutoSerializable;
  testNames[i] = std::string("TestAutoSerializable");

  String *ssas = new String("TestSubAutoSerial");
  Vector<char> *vectsas = new Vector<char>();
  vectsas->push_back('a');
  vectsas->push_back('b');
  vectsas->push_back('c');
  vectsas->push_back('d');
  double *sd = new double;
  *sd = 87.234;

  sentData[++i] = new TestSubAutoSerializable(18446744073709551615ULL,sd,"asdsafsadf",ssas,vectsas,-23412);
  comparFuncs[i] = &compareTestSubAutoSerializable;
  testNames[i] = std::string("TestSubAutoSerializable");

  String *sssas = new String("TestSubSubAutoSerial");
  Vector<char> *vectssas = new Vector<char>();
  vectssas->push_back('a');
  vectssas->push_back('b');
  vectssas->push_back('c');
  vectssas->push_back('d');
  double *ssd = new double;
  *ssd = 87.234;
  sentData[++i] = new TestSubSubAutoSerializable(18446744073709551615ULL,ssd,"asdsaf",sssas,vectssas,-23412,789.3334);
  comparFuncs[i] = &compareTestSubSubAutoSerializable;
  testNames[i] = std::string("TestSubSubAutoSerializable");

}

int main(int argc, char** argv) {
  if (argc == 2) {
    if (std::string(argv[1]) == "--help") {
      
      /*
      Timer timer;
      timer.startTimer();
      usleep(100000);
      time_t sec = 0;
      long nsec = 0;
      timer.getDiffTime(sec,nsec,true);
      std::cout << sec << "," << nsec << std::endl;
      timer.doCheckPoint();
      usleep(100000);
      timer.getDiffTime(sec,nsec,true);
      std::cout << sec << "," << nsec << std::endl;
      timer.getDiffTime(sec,nsec,false);
      std::cout << sec << "," << nsec << std::endl;*/

      /*libcomm::init();
      ConfigLoader *cl = ConfigLoader::getConfigLoader();
      std::cout << *(cl->getParameter("hom-port")) << std::endl; 
      libcomm::clean();


      std::map<std::string, NetAddress> *ips = NetAddress::getInterfaces();
      
      std::map<std::string, NetAddress>::iterator iter = ips->begin();
      for (; iter != ips->end(); ++iter) {
        std::cout << iter->first << " ---->> " << iter->second.getAddress() << "(" << iter->second.getIp() << "," << iter->second.getHostname() << ")" << std::endl;
      }

      delete ips;
      */

      //std::cout << "ip:" << NetAddress::findIp("www.google.ch") << std::endl;
      //std::cout << "hostname:" << NetAddress::findHostname("72.14.221.147") << std::endl;
      //
      /*NetAddress add("www.google.ch",0);
      std::cout << "ip:" << add.getIp() << std::endl;
      std::cout << "hostname:" << add.getHostname() << std::endl;
      std::cout << "address:" << add.getAddress() << std::endl;
*/
      std::cout << "Usage : " << std::endl;
      std::cout << "Test in localhost : without arguments" << std::endl;
      std::cout << "Test with two hopes : " << std::endl;
      std::cout << "  Receiver : test_libcomm port" << std::endl;
      std::cout << "  Sender   : test_libcomm port address" << std::endl;
      exit(0);
    }
   }
  
  LoggerListener *listener = new LoggerConsoleListener();
  listener->setDisplayMessageLevel(DEBUG);
  Logger::addListener("console", listener);
  
  listener = new LoggerFileListener("test_libcomm.log");
  listener->setDisplayMessageLevel(DEBUG);
  Logger::addListener("file", listener);

  libcomm::init();
  
  // Add support for custom classes
  libcomm_structs::addSupportForVector(MyType<Vector<String> >(), false);
  libcomm_structs::addSupportForMap(MyType<uint64_t>(), MyType<Buffer<char>*>(),false,true);
  libcomm::addSupportFor(MyType<TestSerClass>());
  libcomm::addSupportFor(MyType<TestSimpleSerializable>());
  libcomm::addSupportForAutoSerializable(MyType<TestAutoSerializable>());
  libcomm::addSupportForAutoSerializable(MyType<TestSubAutoSerializable>());
  libcomm::addSupportForAutoSerializable(MyType<TestSubSubAutoSerializable>());

  Serializable *sentData[NUMBER_TEST];
  Serializable *receivedData[NUMBER_TEST];
  CompareFunc *compareFuncs = new CompareFunc[NUMBER_TEST];
  std::string testNames[NUMBER_TEST];

  Serializable *sentData_tcp[NUMBER_TEST];
  Serializable *receivedData_tcp[NUMBER_TEST];
  CompareFunc *compareFuncs_tcp = new CompareFunc[NUMBER_TEST];
  std::string testNames_tcp[NUMBER_TEST];

  
  Logger::log(INFO) << "Filling tests data for udp..." << Logger::endmwn("Main");
  fillTestData( (Serializable**) &sentData,
                (CompareFunc*) compareFuncs,
                (std::string*) testNames,
                false);

  Logger::log(INFO) << "Filling tests data for tcp..." << Logger::endmwn("Main");
  fillTestData( (Serializable**) &sentData_tcp,
                (CompareFunc*) compareFuncs_tcp,
                (std::string*) testNames_tcp,
                true);
  
  if (argc == 1) {
    ReceiverThread receiver(  (Serializable**) &receivedData,
                              (Serializable**) &sentData,
                              (CompareFunc*) compareFuncs,
                              (std::string*) testNames,
                              false);

    SenderThread sender((Serializable**) &sentData, false);
    
    Logger::log(INFO) << "Exchanging "<< NUMBER_TEST <<" objects with udp and comparing results..." 
              << Logger::endmwn("Main");
    receiver.start();
    timespec t = {0,500000000};
    nanosleep(&t,0);
    sender.start();

    sender.join();
    receiver.join();
    
    ReceiverThread receiver_tcp(  (Serializable**) &receivedData_tcp,
                              (Serializable**) &sentData_tcp,
                              (CompareFunc*) compareFuncs_tcp,
                              (std::string*) testNames_tcp,
                              true);

    SenderThread sender_tcp((Serializable**) &sentData_tcp, true);
    
    Logger::log(INFO) << "Exchanging "<< NUMBER_TEST <<" objects with tcp and comparing results..." 
              << Logger::endmwn("Main");
    receiver_tcp.start();
    timespec t2 = {0,500000000};
    nanosleep(&t2,0);
    sender_tcp.start();

    sender_tcp.join();
    receiver_tcp.join();

    
  } else if (argc == 2) {
    //Receiver
    int port;
    std::stringstream ss(argv[1]);
    ss >> port;

    ReceiverThread receiver(  (Serializable**) &receivedData,
                              (Serializable**) &sentData,
                              (CompareFunc*) compareFuncs,
                              (std::string*) testNames,
                              port,
                              false);
    Logger::log(INFO) << "Waiting for " << NUMBER_TEST << " objects and comparing results..." 
              << Logger::endmwn("Main");
    receiver.start();
    receiver.join();
  } else if (argc == 3) {
    //Sender
    int port;
    std::stringstream ss(argv[1]);
    ss >> port;
    
    NetAddress address(argv[2], port);
    SenderThread sender((Serializable**) &sentData, &address, false);
    
    Logger::log(INFO) << "Sending " << NUMBER_TEST << " objects..." << Logger::endmwn("Main");
    //Memory leak with sender while using two hopes...too lazy to implement a free method..
    sender.start();
    sender.join();
  }

  delete [] compareFuncs; 
  delete [] compareFuncs_tcp; 
  Logger::log(INFO) << "Done." << Logger::endmwn("Main");
  libcomm::clean();

  return 0; 
}


ReceiverThread::ReceiverThread( Serializable** receivedData,
                    Serializable** sentData,
                    CompareFunc* comparFuncs,
                    std::string *testNames,
                    bool tcp) {
  this->receivedData = receivedData;
  this->comparFuncs = comparFuncs;
  this->testNames = testNames;
  this->sentData = sentData;
  this->tcp = tcp;
  this->port = PORT;
}

ReceiverThread::ReceiverThread( Serializable** receivedData,
                    Serializable** sentData,
                    CompareFunc* comparFuncs,
                    std::string *testNames,
                    bool tcp,
                    int port) {
  this->receivedData = receivedData;
  this->comparFuncs = comparFuncs;
  this->testNames = testNames;
  this->sentData = sentData;
  this->tcp = tcp;
  this->port = port;
}

SenderThread::SenderThread(Serializable** sentData, bool tcp) {
  this->sentData = sentData;
  this->address = (NetAddress*) NULL;
  this->tcp = tcp;
}

SenderThread::SenderThread(Serializable** sentData, NetAddress *address, bool tcp) {
  this->sentData = sentData;
  this->address = address;
  this->tcp = tcp;
}

void *ReceiverThread::run() {
  if (tcp) {
    try {
      TcpServerSocket ssocket(port);
      TcpSocket *socket;
      
      socket = ssocket.acceptConnection();

      NetAddress addr = socket->getLocalAddress();
      Logger::log(INFO) << "Begin receiving data on " << addr.getAddress() 
        << ":" << addr.getPort() << " with tcp" << Logger::endm(this);

      for (int i = 0; i<NUMBER_TEST; ++i) {
        receivedData[i] = socket->readObject();
        printTest(testNames[i],comparFuncs[i](receivedData[i],sentData[i]));
      }
      ssocket.closeServer();
      socket->closeStream();
      delete socket;
    } catch (Exception &e) {
      e.printCodeAndMessage();
    }
  } else {
    UdpSocket socket(port);
    NetAddress addr = socket.getLocalAddress();
    NetAddress addrFrom;
    
    Logger::log(INFO) << "Begin receiving data on " << addr.getAddress() 
      << ":" << addr.getPort() << " with udp" << Logger::endm(this);
    for (int i = 0; i<NUMBER_TEST; ++i) {
      receivedData[i] = socket.readObject(&addrFrom);
      std::cout << "Read object " << i << std::endl;
      printTest(testNames[i],comparFuncs[i](receivedData[i],sentData[i]));
    }
    
    socket.closeStream();
  }
  return NULL;
}

void *SenderThread::run() {
  if (tcp) {
    NetAddress defaultAddress(ADDRESS,PORT); 
    
    if (address == NULL) {
      address = &defaultAddress;
    }
    
    try {
      TcpSocket socket(*address);

      NetAddress addr = socket.getLocalAddress();
      Logger::log(INFO) << "Begin sending data from " << addr.getAddress() 
        << ":" << addr.getPort() << " to " << address->getAddress()
        << ":" << address->getPort() << " with tcp" << Logger::endm(this);
      
      
      for (int i = 0; i<NUMBER_TEST; ++i) {
        socket.writeObject(*sentData[i]);
      }
      
      socket.closeStream();
    } catch (Exception &e) {
      e.printCodeAndMessage();
    }

  } else {
    UdpSocket socket;
    NetAddress defaultAddress(ADDRESS,PORT); 
    NetAddress forGetPort(ADDRESS,1);
    
    NullPlaceholder nullMessage;
    socket.writeObject(nullMessage, forGetPort);
    
    NetAddress addr = socket.getLocalAddress();
    Logger::log(INFO) << "Begin sending data from " << addr.getAddress() 
      << ":" << addr.getPort() << " to " << defaultAddress.getAddress()
      << ":" << defaultAddress.getPort() << " with udp" << Logger::endm(this);
    
    if (address == NULL) {
      address = &defaultAddress;
    }
    
    for (int i = 0; i<NUMBER_TEST; ++i) {
      socket.writeObject(*sentData[i], *address);
    }
    
    socket.closeStream();
  }
    return NULL;
}

ReceiverThread::~ReceiverThread() {
}

SenderThread::~SenderThread() {
}


//Compare funcs
bool compareString(const void* first, const void* second) {
  if ((first == NULL) || (second == NULL)) {
    return false;
  } else {
    String *f = (String*) first;
    String *s = (String*) second;
    bool ok = (*f == *s);
    delete f;
    delete s;
    return ok;
  }
}

bool compareVectorChar(const void* first, const void* second) {
  if ((first == NULL) || (second == NULL)) {
    std::cout << "NULL" << std::endl;
    return false;
  } else {
    Vector<char> *f = (Vector<char>*) first; 
    Vector<char> *s = (Vector<char>*) second; 
    
    bool ok = (*f == *s);
    delete f;
    delete s;
    return ok;
  }
}

bool compareVectorFloat(const void* first, const void* second) {
  if ((first == NULL) || (second == NULL)) {
    return false;
  } else {
    Vector<float> *f = (Vector<float>*) first; 
    Vector<float> *s = (Vector<float>*) second; 
    
    bool ok = (*f == *s);
    delete f;
    delete s;
    return ok;
  }
}

#include <stdio.h>
bool compareVectorDouble(const void* first, const void* second) {
 if ((first == NULL) || (second == NULL)) {
    return false;
  } else {
    Vector<double> *f = (Vector<double>*) first; 
    Vector<double> *s = (Vector<double>*) second; 
    
    bool ok = (*f == *s);
    delete f;
    delete s;
    return ok;
  }
}

bool compareVectorInt8(const void* first, const void* second) {
 if ((first == NULL) || (second == NULL)) {
    return false;
  } else {
    Vector<int8_t> *f = (Vector<int8_t>*) first; 
    Vector<int8_t> *s = (Vector<int8_t>*) second; 
    
    bool ok = (*f == *s);
    delete f;
    delete s;
    return ok;
  }
}

bool compareVectorInt16(const void* first, const void* second) {
 if ((first == NULL) || (second == NULL)) {
    return false;
  } else {
    Vector<int16_t> *f = (Vector<int16_t>*) first; 
    Vector<int16_t> *s = (Vector<int16_t>*) second; 
    
    bool ok = (*f == *s);
    delete f;
    delete s;
    return ok;
  }
}

bool compareVectorInt32(const void* first, const void* second) {
 if ((first == NULL) || (second == NULL)) {
    return false;
  } else {
    Vector<int32_t> *f = (Vector<int32_t>*) first; 
    Vector<int32_t> *s = (Vector<int32_t>*) second; 
    
    bool ok = (*f == *s);
    delete f;
    delete s;
    return ok;
  }
}

bool compareVectorInt64(const void* first, const void* second) {
 if ((first == NULL) || (second == NULL)) {
    return false;
  } else {
    Vector<int64_t> *f = (Vector<int64_t>*) first; 
    Vector<int64_t> *s = (Vector<int64_t>*) second; 
    
    bool ok = (*f == *s);
    delete f;
    delete s;
    return ok;
  }
}

bool compareVectorUInt8(const void* first, const void* second) {
 if ((first == NULL) || (second == NULL)) {
    return false;
  } else {
    Vector<uint8_t> *f = (Vector<uint8_t>*) first; 
    Vector<uint8_t> *s = (Vector<uint8_t>*) second; 
    
    bool ok = (*f == *s);
    delete f;
    delete s;
    return ok;
  }
}

bool compareVectorUInt16(const void* first, const void* second) {
 if ((first == NULL) || (second == NULL)) {
    return false;
  } else {
    Vector<uint16_t> *f = (Vector<uint16_t>*) first; 
    Vector<uint16_t> *s = (Vector<uint16_t>*) second; 
    
    bool ok = (*f == *s);
    delete f;
    delete s;
    return ok;
  }
}

bool compareVectorUInt32(const void* first, const void* second) {
 if ((first == NULL) || (second == NULL)) {
    return false;
  } else {
    Vector<uint32_t> *f = (Vector<uint32_t>*) first; 
    Vector<uint32_t> *s = (Vector<uint32_t>*) second; 
    
    bool ok = (*f == *s);
    delete f;
    delete s;
    return ok;
  }
}

bool compareVectorUInt64(const void* first, const void* second) {
 if ((first == NULL) || (second == NULL)) {
    return false;
  } else {
    Vector<uint64_t> *f = (Vector<uint64_t>*) first; 
    Vector<uint64_t> *s = (Vector<uint64_t>*) second; 
    
    bool ok = (*f == *s);
    delete f;
    delete s;
    return ok;
  }
}

bool compareVectorString(const void* first, const void* second) {
  if ((first == NULL) || (second == NULL)) {
    return false;
  } else {
    Vector<String> *f = (Vector<String>*) first; 
    Vector<String> *s = (Vector<String>*) second; 
    
    bool ok = (*f == *s);
    delete f;
    delete s;
    return ok;
  }
}

bool compareVectorStringPtr(const void* first, const void* second) {
  Vector<String*> *f = (Vector<String*>*) first; 
  Vector<String*> *s = (Vector<String*>*) second; 
  bool returnValue = true;
  if ((first == NULL) || (second == NULL)) {
    return false;
  } else {
    for (size_t i = 0; i<f->size(); ++i) {
      if (*((*f)[i]) != *((*s)[i])) {
        returnValue = false; 
        break;
      }
    }

  }
  for (size_t i = 0; i<f->size(); ++i) {
    delete (*f)[i];
    delete (*s)[i];
  }
  delete f;
  delete s;
  return returnValue;
}


bool compareBufferChars(const void* first, const void* second) {
  Buffer<char> *f = (Buffer<char>*) first;
  Buffer<char> *s = (Buffer<char>*) second;
  bool returnValue = true;
  if ((first == NULL) || (second == NULL)) {
    return false;
  } else if (f->size() != s->size()) {
    return false;
  } else {
    for (size_t i = 0; i<f->size(); ++i) {
      if ((*f)[i] != (*s)[i]) {
        returnValue = false; 
        break;
      }
    }

  }
  delete f;
  delete s;
  return returnValue;
}

bool compareBufferUint64t(const void* first, const void* second) {
  Buffer<uint64_t> *f = (Buffer<uint64_t>*) first;
  Buffer<uint64_t> *s = (Buffer<uint64_t>*) second;
  bool returnValue = true;
  if ((first == NULL) || (second == NULL)) {
    std::cout << "NULL" << std::endl;
    return false;
  } else if (f->size() != s->size()) {
    std::cout << "not same size" << std::endl;
    return false;
  } else {
    for (size_t i = 0; i<f->size(); ++i) {
      if ((*f)[i] != (*s)[i]) {
        std::cout << "("<<i<<")"<< (*f)[i] << "!=" << (*s)[i]<< std::endl;
        returnValue = false; 
        break;
      }
    }

  }
  delete f;
  delete s;
  return returnValue;
}

bool compareMapUin64tBuffer(const void* first, const void* second) {
  Map<uint64_t, Buffer<char>*> *f = (Map<uint64_t, Buffer<char>*>*) first;
  Map<uint64_t, Buffer<char>*> *s = (Map<uint64_t, Buffer<char>*>*) second;
  bool returnValue = true;
  if ((first == NULL) || (second == NULL)) {
    return false;
  } else {
    if (f->size() != s->size()) {
      returnValue = false;
    } else {
      Map<uint64_t, Buffer<char>*>::iterator iter = f->begin();
      for (; iter != f->end(); ++iter) {
        uint64_t k = iter->first;
        Buffer<char> *v = iter->second;
        Map<uint64_t, Buffer<char>*>::iterator iter2 = s->find(k);
        if (iter2 == s->end()) {
          return false;
          break;
        } else {
          uint64_t k2 = iter2->first;
          Buffer<char> *v2 = iter2->second;
          if ((k == k2) || (v->size() == v2->size())) {
            for (size_t i = 0; i<v->size(); ++i) {
              if ((*v)[i] != (*v2)[i]) {
                return false;
              }
            }
          } else {
            return false;
          }
          delete v2;
        }
        delete v;
      }
    }
    delete f;
    delete s;
    return returnValue;
  }
}

bool compareMapCharString(const void* first, const void* second) {
  Map<char,String> *f = (Map<char,String>*) first;
  Map<char,String> *s = (Map<char,String>*) second;
  bool returnValue = true;
  if ((first == NULL) || (second == NULL)) {
    return false;
  } else {
    returnValue = (*f == *s);
    delete f;
    delete s;
    return returnValue;
  }
}

bool compareMapCharStringPtr(const void* first, const void* second) {
  Map<char,String*> *f = (Map<char,String*>*) first;
  Map<char,String*> *s = (Map<char,String*>*) second;
  bool returnValue = true;
  if ((first == NULL) || (second == NULL)) {
    return false;
  } else {
    if (f->size() != s->size()) {
      returnValue = false;
    } else {
      std::map<char,String*>::iterator iter = f->begin();
      for (; iter != f->end(); ++iter) {
        char k = iter->first;
        String* v = iter->second;
        std::map<char,String*>::iterator iter2 = s->find(k);
        if ((iter2 == s->end()) || (*(iter2->second) != *v)) {
          returnValue = false;
          break;
        }
        delete v;
        delete iter2->second;
      }
    }
    delete f;
    delete s;
    return returnValue;
  }
}

bool compareMapStringPtrChar(const void* first, const void* second) {
  Map<String*,char,strPtrCmp> *f = (Map<String*,char,strPtrCmp>*) first;
  Map<String*,char,strPtrCmp> *s = (Map<String*,char,strPtrCmp>*) second;
  bool returnValue = true;
  if ((first == NULL) || (second == NULL)) {
    return false;
  } else {
    if (f->size() != s->size()) {
      returnValue = false;
    } else {
      std::map<String*,char,strPtrCmp>::iterator iter = f->begin();
      std::vector<String*> vect;
      for (; iter != f->end(); ++iter) {
        String* k = iter->first;
        char v = iter->second;
        std::map<String*,char,strPtrCmp>::iterator iter2 = s->find(k);
        if ((iter2 == s->end()) || (iter2->second != v)) {
          returnValue = false;
          break;
        }
        vect.push_back(k);
        vect.push_back(iter2->first);
      }
      for (unsigned int i(0); i<vect.size(); ++i) {
        delete vect[i];
      }
    }
    delete f;
    delete s;
    return returnValue;
  }
}


bool compareMapStringPtrStringPtr(const void* first, const void* second) {
  Map<String*, String*, strPtrCmp> *f = (Map<String*,String*, strPtrCmp>*) first;
  Map<String*, String* ,strPtrCmp> *s = (Map<String*,String*, strPtrCmp>*) second;
  bool returnValue = true;
  if ((first == NULL) || (second == NULL)) {
    return false;
  } else {
    if (f->size() != s->size()) {
      returnValue = false;
    } else {
      std::map<String*,String*,strPtrCmp>::iterator iter = f->begin();
      std::vector<String*> vect;
      for (; iter != f->end(); ++iter) {
        String* k = iter->first;
        String* v = iter->second;
        std::map<String*,String*,strPtrCmp>::iterator iter2 = s->find(k);
        if ((iter2 == s->end()) || ((*(iter2->second)) != *v)) {
          returnValue = false;
          break;
        }
        vect.push_back(k);
        vect.push_back(v);
        vect.push_back(iter2->first);
        vect.push_back(iter2->second);
      }
      for (unsigned int i(0); i<vect.size(); ++i) {
        delete vect[i];
      }
    }
    delete f;
    delete s;
    return returnValue;
  }

}

bool compareTestSerClass(const void* first, const void* second) {
  if ((first == NULL) || (second == NULL)) {
    return false;
  } else {
    TestSerClass *t1 = (TestSerClass*) first;
    TestSerClass *t2 = (TestSerClass*) second;
    bool same = (*t1 == *t2);
    delete t1;
    delete t2;
    return same;
  }
}


bool compareTestSimpleSerializable(const void* first, const void* second) {
  if ((first == NULL) || (second == NULL)) {
    return false;
  } else {
    TestSimpleSerializable *t1 = (TestSimpleSerializable*) first;
    TestSimpleSerializable *t2 = (TestSimpleSerializable*) second;
    bool same = (*t1 == *t2);
    delete t1;
    delete t2;
    return same;
  }
}

bool compareTestAutoSerializable(const void* first, const void* second){
  if ((first == NULL) || (second == NULL)) {
    return false;
  } else {
    TestAutoSerializable *t1 = (TestAutoSerializable*) first;
    TestAutoSerializable *t2 = (TestAutoSerializable*) second;
    bool same = (*t1 == *t2);
    delete t1;
    delete t2;
    return same;
  }
}

bool compareTestSubAutoSerializable(const void* first, const void* second){
  if ((first == NULL) || (second == NULL)) {
    std::cout << "NULL" << std::endl;
    return false;
  } else {
    TestSubAutoSerializable *t1 = (TestSubAutoSerializable*) first;
    TestSubAutoSerializable *t2 = (TestSubAutoSerializable*) second;
    bool same = (*t1 == *t2);
    delete t1;
    delete t2;
    return same;
  }
}

bool compareTestSubSubAutoSerializable(const void* first, const void* second){
  if ((first == NULL) || (second == NULL)) {
    std::cout << "NULL" << std::endl;
    return false;
  } else {
    TestSubSubAutoSerializable *t1 = (TestSubSubAutoSerializable*) first;
    TestSubSubAutoSerializable *t2 = (TestSubSubAutoSerializable*) second;
    bool same = (*t1 == *t2);
    delete t1;
    delete t2;
    return same;
  }
}
