//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_TxnSizeRequest
#define HAZELCAST_TxnSizeRequest

#include "hazelcast/client/txn/BaseTxnRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace queue {
            class HAZELCAST_API TxnSizeRequest : public txn::BaseTxnRequest {
            public:
                TxnSizeRequest(const std::string &name);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

            private:
                std::string name;
            };
        }
    }
}


#endif //HAZELCAST_TxnSizeRequest
