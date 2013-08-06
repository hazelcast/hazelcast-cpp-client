//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "TxnMultiMapGetRequest.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            TxnMultiMapGetRequest::TxnMultiMapGetRequest(const std::string& name, serialization::Data& data)
            : TxnMultiMapRequest(name)
            , data(data) {

            }

            int TxnMultiMapGetRequest::getClassId() const {
                return CollectionPortableHook::TXN_MM_GET;
            }

        }
    }
}
