//auto_serializable_example.h
#ifndef CLIENT_REQUEST_H
#define CLIENT_REQUEST_H

#include <libcomm/structs/auto_serializable.h>
#include <libcomm/structs/string.h>

#define className ClientRequest
class ClientRequest : public AutoSerializable {

  public :
    ClientRequest();

    AUTO_SER_PRIMITIVE_FIELD_WITH_GET_AND_SET(uint16_t, RequestId)
    AUTO_SER_FIELD_WITH_GET_AND_SET(String, RequestContent)
    AUTO_SER_FIELD_PTR_WITH_GET_AND_SET(Serializable, Data)

    AUTO_SER_STUFF(ITEM(Operation), ITEM(RequestContent), ITEM(Data))
};
#undef className

#endif
