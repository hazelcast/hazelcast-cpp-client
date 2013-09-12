//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_TxnMultiMapRequest
#define HAZELCAST_TxnMultiMapRequest

#include "Request.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace multimap {
            class TxnMultiMapRequest : public impl::Request {
            public:
                TxnMultiMapRequest(const std::string& name);

                int getFactoryId() const;

                void writePortable(serialization::PortableWriter& writer) const;

            private:
                std::string name;
            };

        }
    }
}
#endif //HAZELCAST_TxnMultiMapRequest
