//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_QUEUE_OFFER_REQUEST
#define HAZELCAST_QUEUE_OFFER_REQUEST

#include "hazelcast/client/impl/PortableRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class Data;
        }
        namespace queue {
            class HAZELCAST_API OfferRequest : public impl::PortableRequest {
            public:
                OfferRequest(const std::string& name, serialization::Data& data, long timeout);

                OfferRequest(const std::string& name, serialization::Data& data);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter& writer) const;



            private:
                serialization::Data& data;
                const std::string& name;
                long timeoutInMillis;
            };
        }
    }
}

#endif //HAZELCAST_OFFER_REQUEST
