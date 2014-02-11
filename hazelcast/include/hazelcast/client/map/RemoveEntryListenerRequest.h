//
// Created by sancar koyunlu on 07/01/14.
//


#ifndef HAZELCAST_RemoveEntryListenerRequest
#define HAZELCAST_RemoveEntryListenerRequest

#include "hazelcast/client/impl/BaseRemoveListenerRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace map {

            class HAZELCAST_API RemoveEntryListenerRequest : public impl::BaseRemoveListenerRequest {
            public:
                RemoveEntryListenerRequest(const std::string &name, const std::string &registrationId);

                int getFactoryId() const;

                int getClassId() const;
            };

        }
    }
}


#endif //HAZELCAST_RemoveEntryListenerRequest
