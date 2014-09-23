//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_QUEUE_OFFER_REQUEST
#define HAZELCAST_QUEUE_OFFER_REQUEST

#include "hazelcast/client/impl/ClientRequest.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace queue {
            class HAZELCAST_API OfferRequest : public impl::ClientRequest {
            public:
                OfferRequest(const std::string &name, serialization::pimpl::Data &data, long timeout);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

            private:
                std::string name;
                long timeoutInMillis;
                serialization::pimpl::Data data;
            };
        }
    }
}

#endif //HAZELCAST_OFFER_REQUEST

