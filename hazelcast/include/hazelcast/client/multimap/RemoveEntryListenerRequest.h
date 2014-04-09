//
// Created by sancar koyunlu on 15/01/14.
//


#ifndef HAZELCAST_MultiMap_RemoveEntryListenerRequest
#define HAZELCAST_MultiMap_RemoveEntryListenerRequest

#include "hazelcast/client/impl/BaseRemoveListenerRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace multimap {
            class HAZELCAST_API RemoveEntryListenerRequest : public impl::BaseRemoveListenerRequest {
            public:
                RemoveEntryListenerRequest(const std::string &name, const std::string &registrationId);

                int getFactoryId() const;

                int getClassId() const;

                bool isRetryable() const;

            };
        }
    }
}


#endif //HAZELCAST_MultiMap_RemoveEntryListenerRequest

