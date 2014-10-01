//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_TxnMultiMapSizeRequest
#define HAZELCAST_TxnMultiMapSizeRequest

#include "hazelcast/client/multimap/TxnMultiMapRequest.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            class TxnMultiMapSizeRequest : public TxnMultiMapRequest {
            public:
                TxnMultiMapSizeRequest(const std::string& name);

                int getClassId() const;
            };

        }
    }
}
#endif //HAZELCAST_TxnMultiMapSizeRequest

