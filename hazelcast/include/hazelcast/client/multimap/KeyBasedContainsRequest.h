//
// Created by sancar koyunlu on 18/08/14.
//


#ifndef HAZELCAST_KeyBasedContainsRequest
#define HAZELCAST_KeyBasedContainsRequest


#include "hazelcast/client/multimap/KeyBasedRequest.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            class KeyBasedContainsRequest : public KeyBasedRequest {
            public:
                KeyBasedContainsRequest(const std::string& name, const serialization::pimpl::Data& key
                , const serialization::pimpl::Data& value, long threadId);

                KeyBasedContainsRequest(const std::string& name, const serialization::pimpl::Data& key, long threadId);

                void write(serialization::PortableWriter& writer) const;

                int getClassId() const;

                bool isRetryable() const;

            private:
                bool hasValue;
                serialization::pimpl::Data value;
                long threadId;
            };
        }
    }
}


#endif //HAZELCAST_KeyBasedContainsRequest
