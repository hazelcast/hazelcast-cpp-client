//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_TxnSetAddRequest
#define HAZELCAST_TxnSetAddRequest

#include "hazelcast/client/collection/TxnCollectionRequest.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            class TxnSetAddRequest : public TxnCollectionRequest {
            public:
                TxnSetAddRequest(const std::string&name, serialization::Data *);

                int getClassId() const;

            };

        }
    }
}
#endif //HAZELCAST_TxnSetAddRequest
