//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_TxnMultiMapValueCountRequest
#define HAZELCAST_TxnMultiMapValueCountRequest

#include "TxnMultiMapRequest.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            class Data;
        }
        namespace multimap {
            class TxnMultiMapValueCountRequest : public TxnMultiMapRequest {
            public:
                TxnMultiMapValueCountRequest(const std::string& name, const serialization::Data& data);

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;

            private:
                const serialization::Data& data;
            };

        }
    }
}
#endif //HAZELCAST_TxnMultiMapValueCountRequest
