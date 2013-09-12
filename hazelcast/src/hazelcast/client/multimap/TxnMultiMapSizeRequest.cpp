//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "TxnMultiMapSizeRequest.h"
#include "MultiMapPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            TxnMultiMapSizeRequest::TxnMultiMapSizeRequest(const std::string& name)
            : TxnMultiMapRequest(name) {

            }

            int TxnMultiMapSizeRequest::getClassId() const {
                return MultiMapPortableHook::TXN_MM_SIZE;
            }


        }
    }
}