//
// Created by sancar koyunlu on 8/2/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_CommitTxnRequest
#define HAZELCAST_CommitTxnRequest

#include "hazelcast/client/txn/BaseTxnRequest.h"

namespace hazelcast {
    namespace client {
        namespace txn {
            class CommitTxnRequest : public BaseTxnRequest {
            public:
                CommitTxnRequest(bool prepareAndCommit);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter& writer) const;

            private:
                bool prepareAndCommit;
            };
        }
    }
}

#endif //HAZELCAST_CommitTxnRequest

