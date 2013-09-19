//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_TxnSetSizeRequest
#define HAZELCAST_TxnSetSizeRequest

#include "TxnCollectionRequest.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            class TxnSetSizeRequest : public TxnCollectionRequest {
            public:
                TxnSetSizeRequest(const std::string& name);

                int getClassId() const;
            };

        }
    }
}

#endif //HAZELCAST_TxnSetSizeRequest
