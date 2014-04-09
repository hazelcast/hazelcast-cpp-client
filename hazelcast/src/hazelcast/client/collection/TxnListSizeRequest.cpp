//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/collection/TxnListSizeRequest.h"
#include "hazelcast/client/collection/CollectionPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            TxnListSizeRequest::TxnListSizeRequest(const std::string& name)
            :TxnCollectionRequest(name) {

            }

            int TxnListSizeRequest::getClassId() const {
                return CollectionPortableHook::TXN_LIST_SIZE;
            }
        }
    }
}
