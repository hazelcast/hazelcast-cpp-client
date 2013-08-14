//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "TxnMultiMapValueCountRequest.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            TxnMultiMapValueCountRequest::TxnMultiMapValueCountRequest(const std::string& name,  serialization::Data& data)
            : TxnMultiMapRequest(name)
            , data(data) {

            }

            int TxnMultiMapValueCountRequest::getClassId() const {
                return CollectionPortableHook::TXN_MM_VALUE_COUNT;
            }

        }
    }
}