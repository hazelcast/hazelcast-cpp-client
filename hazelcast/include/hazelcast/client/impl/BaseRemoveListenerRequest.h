//
// Created by sancar koyunlu on 11/02/14.
//


#ifndef HAZELCAST_BaseRemoveListenerRequest
#define HAZELCAST_BaseRemoveListenerRequest

#include "hazelcast/client/impl/PortableRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class PortableWriter;
        }
        namespace impl {
            class BaseRemoveListenerRequest : public PortableRequest {
            public:
                BaseRemoveListenerRequest(const std::string &name, const std::string registrationId);

                void setRegistrationId(const std::string &registrationId);

                virtual void write(serialization::PortableWriter &writer) const;

            private:
                std::string name;
                std::string registrationId;
            };

        }
    }
}

#endif //HAZELCAST_BaseRemoveListenerRequest
