//auto_serializable_example.cpp
#include "auto_serializable_example.h"

#define className ClientRequest
AUTO_SER_STATIC_STUFF
#undef className

ClientRequest::ClientRequest() {}
