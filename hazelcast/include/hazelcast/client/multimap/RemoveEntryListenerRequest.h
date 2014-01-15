//
// Created by sancar koyunlu on 15/01/14.
//


#ifndef HAZELCAST_MultiMap_RemoveEntryListenerRequest
#define HAZELCAST_MultiMap_RemoveEntryListenerRequest

#include "hazelcast/client/impl/RetryableRequest.h"
#include "hazelcast/client/multimap/AllPartitionsRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace multimap {
            class HAZELCAST_API RemoveEntryListenerRequest : public AllPartitionsRequest, public RetryableRequest {
            public:
                RemoveEntryListenerRequest(const std::string &name, const std::string &registrationId);

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

            private:
                std::string registrationId;

            };
        }
    }
}


#endif //HAZELCAST_MultiMap_RemoveEntryListenerRequest
