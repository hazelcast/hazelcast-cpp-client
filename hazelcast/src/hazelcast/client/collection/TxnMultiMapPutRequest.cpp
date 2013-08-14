//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "TxnMultiMapPutRequest.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            TxnMultiMapPutRequest::TxnMultiMapPutRequest(const std::string& name, serialization::Data& key, serialization::Data& value)
            : TxnMultiMapRequest(name)
            , key(key)
            , value(value) {

            }

            int TxnMultiMapPutRequest::getClassId() const {
                return CollectionPortableHook::TXN_MM_PUT;
            }


        }
    }
}