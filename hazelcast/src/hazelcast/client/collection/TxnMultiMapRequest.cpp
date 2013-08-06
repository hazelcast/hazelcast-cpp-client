//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "TxnMultiMapRequest.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            TxnMultiMapRequest::TxnMultiMapRequest(const std::string& name)
            :name(name) {

            }

            int TxnMultiMapRequest::getFactoryId() const {
                return CollectionPortableHook::F_ID;
            }

        }
    }
}