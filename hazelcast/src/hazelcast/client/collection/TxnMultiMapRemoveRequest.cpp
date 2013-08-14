//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "TxnMultiMapRemoveRequest.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            TxnMultiMapRemoveRequest::TxnMultiMapRemoveRequest(const std::string& name, serialization::Data& key)
            : TxnMultiMapRequest(name)
            , key(key)
            , value(NULL) {

            }

            TxnMultiMapRemoveRequest::TxnMultiMapRemoveRequest(const std::string& name, serialization::Data& key, serialization::Data *value)
            : TxnMultiMapRequest(name)
            , key(key)
            , value(value) {

            }


            int TxnMultiMapRemoveRequest::getClassId() const {
                return CollectionPortableHook::TXN_MM_REMOVE;
            }

        }
    }
}