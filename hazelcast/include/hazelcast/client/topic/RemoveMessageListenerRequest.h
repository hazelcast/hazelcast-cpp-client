//
// Created by sancar koyunlu on 15/01/14.
//


#ifndef HAZELCAST_RemoveMessageListenerRequest
#define HAZELCAST_RemoveMessageListenerRequest


#include "hazelcast/client/impl/BaseRemoveListenerRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace topic {
            class HAZELCAST_API RemoveMessageListenerRequest : public impl::BaseRemoveListenerRequest {
            public:
                RemoveMessageListenerRequest(const std::string &instanceName, const std::string &registrationId);

                int getFactoryId() const;

                int getClassId() const;
            };
        }
    }
}


#endif //HAZELCAST_RemoveMessageListenerRequest

