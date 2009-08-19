//auto_sub_serializable_example.h
#ifndef SPECIAL_CLIENT_REQUEST_H
#define SPECIAL_CLIENT_REQUEST_H

#include "auto_serializable_example.h"

#define className ClientRequest
class SpecialClientRequest : public ClientRequest {

  public :
    SpecialClientRequest();

    AUTO_SER_PRIMITIVE_FIELD_WITH_GET_AND_SET(uint64_t, TimeStamp)

    AUTO_SER_STUFF_WITH_SUPERCLASS(ClientRequest, ITEM(TimeStamp))
};
#undef className

#endif
