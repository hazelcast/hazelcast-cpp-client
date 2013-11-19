//
// Created by sancar koyunlu on 8/2/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_CommitTxnRequest
#define HAZELCAST_CommitTxnRequest

#include "hazelcast/client/impl/PortableRequest.h"

namespace hazelcast {
    namespace client {
        namespace txn {
            class CommitTxnRequest : public impl::PortableRequest {
            public:

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter &writer) const;
            };
        }
    }
}

#endif //HAZELCAST_CommitTxnRequest
