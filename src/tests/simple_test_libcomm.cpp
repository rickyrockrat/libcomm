#include <libcomm/libcomm.h>
#include <libcomm/structs/string_serializable.h>
#include <libcomm/structs/buffer_serializable.h>
#include <libcomm/structs/auto_serializable.h>
#include <libcomm/structs/vector_serializable.h>
#include <libcomm/file.h>
#include <iostream>
#include <iomanip>

#define className TestClass
class TestClass : public AutoSerializable {
  public:  
    TestClass(void) {
    }
    TestClass(uint32_t id): Id(id) {
    }
    ~TestClass(void) {
      delete Buff;
    }

    AUTO_SER_PRIMITIVE_FIELD_WITH_GET(uint32_t, Id)
    AUTO_SER_PRIMITIVE_FIELD_WITH_GET_AND_SET(uint64_t, UserData)
    AUTO_SER_FIELD_PTR_WITH_GET_AND_SET(Buffer<char>, Buff)
    AUTO_SER_FIELD_WITH_GET_AND_SET(String, Str)
    AUTO_SER_FIELD_WITH_GET_AND_SET(Vector<uint32_t>, Vect)

    AUTO_SER_STUFF(ITEM(Id), ITEM(UserData), ITEM(Str), ITEM(Buff), ITEM(Vect))
};
AUTO_SER_STATIC_STUFF
#undef className


void first_test(void) {
  SerializationManager *sm;
  sm = SerializationManager::getSerializationManager();
  String str("Bonjour je m'appelle Alex");
  String *clone;

  clone = (String*) sm->clone(str);

  std::cout << "Str      :" << str << std::endl;
  std::cout << "Clone str:" << *clone << std::endl;
  
  delete clone;

  Buffer<char> buffer(3);
  buffer[0] = 'a';
  buffer[1] = 'b';
  buffer[2] = 'c';
  
  Buffer<char> *buffer_clone = (Buffer<char>*) sm->clone(buffer);
  
  std::cout << "First Second" << std::endl;
  for (size_t i = 0; i<buffer.size(); ++i) {
    std::cout << buffer[i] << " " << (*buffer_clone)[i] << std::endl;
  }
  
  delete buffer_clone;

  String *str1 = new String("asdfasdf", 4);
  std::cout << *str1 << std::endl;
  delete str1;
}

void second_test(void) {
  Buffer<char> buffer(3);
  buffer[0] = 'a';
  buffer[1] = 'b';
  buffer[2] = 'c';
  try {
    File f("asdf2", File::rw, File::create | File::trunc); 
    f.writeObject(buffer);
    f.closeStream();
  } catch (Exception &e) {
    e.printCodeAndMessage();
  }
}

void third_test(void) {
  Buffer<char> *buff;
  try {
    File f("asdf", File::r); 
    buff = (Buffer<char>*) f.readObject();
    f.closeStream();

    for (size_t i = 0; i<buff->size(); ++i) {
      std::cout << (*buff)[i] << " ";
    }
    std::cout << std::endl;

  } catch (Exception &e) {
    std::cout << File::exists("asdf") << std::endl;
    e.printCodeAndMessage();
  }
}

void fourth_test(void) {
  SerializationManager *sm;
  sm = SerializationManager::getSerializationManager();

  TestClass tc(156);
  tc.setUserData(78);
  tc.setStr("Prout");
  Buffer<char> *buffer = new Buffer<char>(3);
  (*buffer)[0] = 'a';
  (*buffer)[1] = 'b';
  (*buffer)[2] = 'c';
  tc.setBuff(buffer);
  Vector<uint32_t> vect;
  vect.push_back(1);
  vect.push_back(2);
  vect.push_back(3);
  vect.push_back(4);
  tc.setVect(vect);

  File f("asdf", File::rw, File::create | File::trunc); 
  f.writeObject(tc);
  f.closeStream();
  
  TestClass *clone = (TestClass*) sm->clone(tc);
  std::cout << "id:" << clone->getId() << "; userData:" << clone->getUserData() 
  << "; str:" << clone->getStr() << "; buff: ";
  const Buffer<char> *buff = clone->getBuff();
  for (size_t i = 0; i<buff->size(); ++i) {
    std::cout << (*buff)[i] << " ";
  }
  const Vector<uint32_t> &vec = clone->getVect();
  std::cout << "; vec(vec.size=" <<vec.size() << ":";
  for (size_t i = 0; i<vec.size(); ++i) {
    std::cout << vec[i] << " ";
  }
  std::cout << std::endl;
  
  delete clone;
}

int main(void) {
  
  libcomm::init();
  libcomm::addSupportForAutoSerializable(MyType<TestClass>());
  
  //first_test();
  //second_test();
  //third_test();
  fourth_test();

  libcomm::clean();

  return 0;
}
