//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_TxnMultiMapValueCountRequest
#define HAZELCAST_TxnMultiMapValueCountRequest

#include "hazelcast/client/multimap/TxnMultiMapRequest.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl{
                class Data;
            }

        }
        namespace multimap {
            class HAZELCAST_API TxnMultiMapValueCountRequest : public TxnMultiMapRequest {
            public:
                TxnMultiMapValueCountRequest(const std::string &name, const serialization::pimpl::Data &data);

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

            private:
                serialization::pimpl::Data data;
            };

        }
    }
}
#endif //HAZELCAST_TxnMultiMapValueCountRequest
