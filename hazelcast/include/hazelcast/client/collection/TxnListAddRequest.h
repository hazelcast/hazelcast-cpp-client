//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_TxnListAddRequest
#define HAZELCAST_TxnListAddRequest

#include "hazelcast/client/collection/TxnCollectionRequest.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            class TxnListAddRequest : public TxnCollectionRequest {
            public:
                TxnListAddRequest(const std::string& name, const serialization::pimpl::Data&);

                int getClassId() const;

            };
        }
    }
}

#endif //HAZELCAST_TxnListAddRequest

