//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_TxnMultiMapGetRequest
#define HAZELCAST_TxnMultiMapGetRequest

#include "hazelcast/client/multimap/TxnMultiMapRequest.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            class HAZELCAST_API TxnMultiMapGetRequest : public TxnMultiMapRequest {
            public:
                TxnMultiMapGetRequest(const std::string &name, const serialization::pimpl::Data &data);

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

            private:
                serialization::pimpl::Data data;
            };

        }
    }
}
#endif //HAZELCAST_TxnMultiMapGetRequest

