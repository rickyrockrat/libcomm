#include "test_libcomm_testautoser.h"

#define className TestAutoSerializable
AUTO_SER_STATIC_STUFF 
#undef className

TestAutoSerializable::TestAutoSerializable( uint64_t a, double *b, String str, 
 String *strPtr, Vector<char> *v) :a(a),b(b),str(str),strPtr(strPtr),v(v) {}

TestAutoSerializable::~TestAutoSerializable() {
  delete strPtr;
  delete b;
  delete v;
}

TestAutoSerializable::TestAutoSerializable(){}

void TestAutoSerializable::copyFields(const TestAutoSerializable &t) {
  seta(t.geta());
  double *be = new double;
  *be = *t.getb();
  setb(be);
  setstr(t.getstr());
  String *strPtre = new String();
  *strPtre = *(t.getstrPtr());
  setstrPtr(strPtre);
  Vector<char> *ve = new Vector<char>();
  *ve = *(t.getv());
  setv(ve);
}

bool TestAutoSerializable::operator==(TestAutoSerializable &tas) {
  return ((a == tas.geta()) && (*strPtr == *(tas.getstrPtr())) && (*b == *(tas.getb())) && //  &&
          (str == tas.getstr())) && (*v == *(tas.v));
}
