//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_TxnCollectionRequest
#define HAZELCAST_TxnCollectionRequest

#include "PortableRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class Data;
        }
        namespace collection {
            class TxnCollectionRequest : public impl::PortableRequest {
            public:
                TxnCollectionRequest(const std::string &name);

                TxnCollectionRequest(const std::string &name, serialization::Data *);

                int getFactoryId() const;

                void writePortable(serialization::PortableWriter &writer) const;

            private:
                const std::string &name;
                serialization::Data *data;
            };
        }
    }
}


#endif //HAZELCAST_TxnCollectionRequest
