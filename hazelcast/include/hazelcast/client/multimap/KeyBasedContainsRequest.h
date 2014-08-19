//
// Created by sancar koyunlu on 18/08/14.
//


#ifndef HAZELCAST_KeyBasedContainsRequest
#define HAZELCAST_KeyBasedContainsRequest


#include "hazelcast/client/multimap/KeyBasedRequest.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            class HAZELCAST_API KeyBasedContainsRequest : public KeyBasedRequest {
            public:
                KeyBasedContainsRequest(const std::string& name, const serialization::pimpl::Data& key
                , const serialization::pimpl::Data& value);

                KeyBasedContainsRequest(const std::string& name, const serialization::pimpl::Data& key);

                void write(serialization::PortableWriter& writer) const ;

                int getClassId() const;

                bool isRetryable() const;

            private:
                bool hasValue;
                serialization::pimpl::Data value;
            };
        }
    }
}


#endif //HAZELCAST_KeyBasedContainsRequest
