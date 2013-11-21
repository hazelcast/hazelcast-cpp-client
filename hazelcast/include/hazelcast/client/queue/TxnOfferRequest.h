//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_TxnOfferRequest
#define HAZELCAST_TxnOfferRequest

#include "hazelcast/client/impl/PortableRequest.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/PortableReader.h"
#include "hazelcast/client/serialization/Data.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace queue {
            class TxnOfferRequest : public impl::PortableRequest {
            public:
                TxnOfferRequest(const std::string &name, long timeoutInMillis, serialization::Data &);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter &writer) const;

            private:
                const std::string& name;
                long timeoutInMillis;
                serialization::Data &data;
            };
        }
    }
}

#endif //HAZELCAST_TxnOfferRequest
