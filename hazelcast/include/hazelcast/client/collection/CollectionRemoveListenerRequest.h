//
// Created by sancar koyunlu on 15/01/14.
//


#ifndef HAZELCAST_CollectionRemoveListenerRequest
#define HAZELCAST_CollectionRemoveListenerRequest

#include "hazelcast/client/collection/CollectionRequest.h"
#include "hazelcast/client/impl/BaseRemoveListenerRequest.h"


namespace hazelcast {
    namespace client {
        namespace collection {
            class HAZELCAST_API CollectionRemoveListenerRequest : public impl::BaseRemoveListenerRequest {
            public:
                CollectionRemoveListenerRequest(const std::string &name, const std::string &serviceName, const std::string &registrationId);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

            private:
                std::string serviceName;

            };
        }
    }
}

#endif //HAZELCAST_CollectionRemoveListenerRequest

