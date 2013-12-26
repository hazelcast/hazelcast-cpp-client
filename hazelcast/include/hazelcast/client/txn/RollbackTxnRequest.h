//
// Created by sancar koyunlu on 8/2/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_RollbackTransactionRequest
#define HAZELCAST_RollbackTransactionRequest

#include "hazelcast/client/impl/PortableRequest.h"

namespace hazelcast {
    namespace client {
        namespace txn {
            class HAZELCAST_API RollbackTxnRequest : public impl::PortableRequest {
            public:
                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

            };
        }
    }
}


#endif //HAZELCAST_RollbackTransactionRequest
