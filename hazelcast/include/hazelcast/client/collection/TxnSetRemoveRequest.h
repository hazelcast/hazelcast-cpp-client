//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_TxnSetRemoveRequest
#define HAZELCAST_TxnSetRemoveRequest

#include "hazelcast/client/collection/TxnCollectionRequest.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            class TxnSetRemoveRequest : public TxnCollectionRequest {
            public:
                TxnSetRemoveRequest(const std::string &name, serialization::pimpl::Data &);

                int getClassId() const;
            };
        }
    }
}


#endif //HAZELCAST_TxnSetRemoveRequest

