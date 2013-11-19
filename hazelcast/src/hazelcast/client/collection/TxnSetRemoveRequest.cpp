//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "TxnSetRemoveRequest.h"
#include "hazelcast/client/collection/CollectionPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            TxnSetRemoveRequest::TxnSetRemoveRequest(const std::string& name, serialization::Data *data)
            :TxnCollectionRequest(name, data) {

            }

            int TxnSetRemoveRequest::getClassId() const {
                return CollectionPortableHook::TXN_SET_REMOVE;
            }

        }
    }
}