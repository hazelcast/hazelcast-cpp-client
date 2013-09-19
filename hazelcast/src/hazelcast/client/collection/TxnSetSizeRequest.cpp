//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "TxnSetSizeRequest.h"
#include "CollectionPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            TxnSetSizeRequest::TxnSetSizeRequest(const std::string& name)
            :TxnCollectionRequest(name) {

            }

            int TxnSetSizeRequest::getClassId() const {
                return CollectionPortableHook::TXN_SET_SIZE;
            }

        }
    }
}