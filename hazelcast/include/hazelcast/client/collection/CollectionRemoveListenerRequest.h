//
// Created by sancar koyunlu on 15/01/14.
//


#ifndef HAZELCAST_CollectionRemoveListenerRequest
#define HAZELCAST_CollectionRemoveListenerRequest

#include "hazelcast/client/collection/CollectionRequest.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            class HAZELCAST_API CollectionRemoveListenerRequest : public CollectionRequest {
            public:
                CollectionRemoveListenerRequest(const std::string &name, const std::string &serviceName, const std::string &registrationId);

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

            private:
                std::string registrationId;

            };
        }
    }
}

#endif //HAZELCAST_CollectionRemoveListenerRequest
