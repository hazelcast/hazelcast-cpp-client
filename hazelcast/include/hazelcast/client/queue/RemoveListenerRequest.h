//
// Created by sancar koyunlu on 15/01/14.
//


#ifndef HAZELCAST_RemoveListenerRequest
#define HAZELCAST_RemoveListenerRequest

#include "hazelcast/client/impl/PortableRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace queue {
            class HAZELCAST_API RemoveListenerRequest : public impl::PortableRequest {
            public:
                RemoveListenerRequest(const std::string &name, const std::string &registrationId);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

            private:
                std::string name;
                std::string registrationId;
            };
        }
    }
}

#endif //HAZELCAST_RemoveListenerRequest
