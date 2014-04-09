//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_TxnMultiMapPutRequest
#define HAZELCAST_TxnMultiMapPutRequest

#include "hazelcast/client/multimap/TxnMultiMapRequest.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            class HAZELCAST_API TxnMultiMapPutRequest : public TxnMultiMapRequest {
            public:
                TxnMultiMapPutRequest(const std::string &name, const serialization::pimpl::Data &key, const serialization::pimpl::Data &value);

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

            private:
                serialization::pimpl::Data key;
                serialization::pimpl::Data value;
            };

        }
    }
}
#endif //HAZELCAST_TxnMultiMapPutRequest

