//
// Created by sancar koyunlu on 15/01/14.
//


#ifndef HAZELCAST_RemoveListenerRequest
#define HAZELCAST_RemoveListenerRequest

#include "hazelcast/client/impl/BaseRemoveListenerRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace queue {
            class HAZELCAST_API RemoveListenerRequest : public impl::BaseRemoveListenerRequest {
            public:
                RemoveListenerRequest(const std::string &name, const std::string &registrationId);

                int getFactoryId() const;

                int getClassId() const;

            };
        }
    }
}

#endif //HAZELCAST_RemoveListenerRequest
