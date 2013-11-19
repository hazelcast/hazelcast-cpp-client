//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_TxnListRemoveRequest
#define HAZELCAST_TxnListRemoveRequest

#include "hazelcast/client/collection/TxnCollectionRequest.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            class TxnListRemoveRequest : public TxnCollectionRequest {
            public:
                TxnListRemoveRequest(const std::string&name, serialization::Data *);

                int getClassId() const;

            };
        }
    }
}

#endif //HAZELCAST_TxnListRemoveRequest
