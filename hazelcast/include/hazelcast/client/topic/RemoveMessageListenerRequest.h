//
// Created by sancar koyunlu on 15/01/14.
//


#ifndef HAZELCAST_RemoveMessageListenerRequest
#define HAZELCAST_RemoveMessageListenerRequest


#include "hazelcast/client/impl/PortableRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace topic {
            class HAZELCAST_API RemoveMessageListenerRequest : public impl::PortableRequest {
            public:
                RemoveMessageListenerRequest(const std::string &instanceName, const std::string &registrationId);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

            private:
                std::string instanceName;
                std::string registrationId;
            };
        }
    }
}


#endif //HAZELCAST_RemoveMessageListenerRequest
