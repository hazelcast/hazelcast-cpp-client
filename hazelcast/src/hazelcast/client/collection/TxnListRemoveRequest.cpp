//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "TxnListRemoveRequest.h"
#include "CollectionPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            TxnListRemoveRequest::TxnListRemoveRequest(const std::string& name, serialization::Data *data)
            :TxnCollectionRequest(name, data) {

            }

            int TxnListRemoveRequest::getClassId() const {
                return CollectionPortableHook::TXN_LIST_REMOVE;
            }
        }
    }
}