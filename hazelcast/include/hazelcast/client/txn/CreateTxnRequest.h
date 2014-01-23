//
// Created by sancar koyunlu on 8/2/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_CreateTxnRequest
#define HAZELCAST_CreateTxnRequest

#include "hazelcast/client/TransactionOptions.h"
#include "hazelcast/client/txn/BaseTxnRequest.h"

namespace hazelcast {
    namespace client {
        namespace txn {
            class HAZELCAST_API CreateTxnRequest : public BaseTxnRequest {
            public:
                CreateTxnRequest(TransactionOptions options);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

            private:
                TransactionOptions options;
            };
        }
    }
}


#endif //HAZELCAST_CreateTxnRequest
