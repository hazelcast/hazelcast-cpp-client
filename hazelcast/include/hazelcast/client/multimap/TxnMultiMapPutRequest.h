//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_TxnMultiMapPutRequest
#define HAZELCAST_TxnMultiMapPutRequest

#include "TxnMultiMapRequest.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            class Data;
        }
        namespace multimap {
            class TxnMultiMapPutRequest : public TxnMultiMapRequest {
            public:
                TxnMultiMapPutRequest(const std::string& name, const serialization::Data& key, const serialization::Data& value);

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;

            private:
                const serialization::Data& key;
                const serialization::Data& value;
            };

        }
    }
}
#endif //HAZELCAST_TxnMultiMapPutRequest
