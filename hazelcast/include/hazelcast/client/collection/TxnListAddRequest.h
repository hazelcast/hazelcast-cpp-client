//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_TxnListAddRequest
#define HAZELCAST_TxnListAddRequest

#include "TxnCollectionRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace collection {
            class TxnListAddRequest : public TxnCollectionRequest{
            public:
                TxnListAddRequest(const std::string&name, serialization::Data *);

                int getClassId() const;

            };
        }
    }
}

#endif //HAZELCAST_TxnListAddRequest
