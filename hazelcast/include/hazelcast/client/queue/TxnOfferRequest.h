//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_TxnOfferRequest
#define HAZELCAST_TxnOfferRequest

#include "hazelcast/client/txn/BaseTxnRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class Data;
        }
        namespace queue {
            class HAZELCAST_API TxnOfferRequest : public txn::BaseTxnRequest {
            public:
                TxnOfferRequest(const std::string &name, long timeoutInMillis, serialization::Data &);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

            private:
                const std::string &name;
                long timeoutInMillis;
                serialization::Data &data;
            };
        }
    }
}

#endif //HAZELCAST_TxnOfferRequest
