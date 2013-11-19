//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "TxnSetAddRequest.h"
#include "hazelcast/client/collection/CollectionPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            TxnSetAddRequest::TxnSetAddRequest(const std::string& name, serialization::Data *data)
            :TxnCollectionRequest(name, data) {

            }

            int TxnSetAddRequest::getClassId() const {
                return CollectionPortableHook::TXN_SET_ADD;
            }

        }
    }
}