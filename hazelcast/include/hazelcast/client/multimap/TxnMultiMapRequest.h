//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_TxnMultiMapRequest
#define HAZELCAST_TxnMultiMapRequest

#include "hazelcast/client/txn/BaseTxnRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace multimap {
            class HAZELCAST_API TxnMultiMapRequest : public txn::BaseTxnRequest {
            public:
                TxnMultiMapRequest(const std::string &name);

                int getFactoryId() const;

                void write(serialization::PortableWriter &writer) const;

            private:
                std::string name;
            };

        }
    }
}
#endif //HAZELCAST_TxnMultiMapRequest
