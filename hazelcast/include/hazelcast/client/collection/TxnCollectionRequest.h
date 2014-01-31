//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_TxnCollectionRequest
#define HAZELCAST_TxnCollectionRequest

#include "hazelcast/client/txn/BaseTxnRequest.h"
#include "hazelcast/client/serialization/Data.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace collection {
            class HAZELCAST_API TxnCollectionRequest : public txn::BaseTxnRequest {
            public:
                TxnCollectionRequest(const std::string &name);

                TxnCollectionRequest(const std::string &name, serialization::Data &);

                int getFactoryId() const;

                void write(serialization::PortableWriter &writer) const;

            private:
                std::string name;
                bool hasData;
                serialization::Data data;
            };
        }
    }
}


#endif //HAZELCAST_TxnCollectionRequest
