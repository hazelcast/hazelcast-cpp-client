//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_TxnPollRequest
#define HAZELCAST_TxnPollRequest

#include "hazelcast/client/impl/PortableRequest.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace queue {
            class HAZELCAST_API TxnPollRequest : public impl::PortableRequest {
            public:
                TxnPollRequest(const std::string &name, long timeout);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

            private:
                const std::string& name;
                long timeout;
            };
        }
    }
}


#endif //HAZELCAST_TxnPollRequest
