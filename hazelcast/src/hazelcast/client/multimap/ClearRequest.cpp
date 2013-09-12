//
// Created by sancar koyunlu on 9/10/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/multimap/ClearRequest.h"
#include "MultiMapPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            ClearRequest::ClearRequest(const std::string& name)
            : AllPartitionsRequest(name) {

            }

            int ClearRequest::getClassId() const {
                return MultiMapPortableHook::CLEAR;
            }

        }
    }
}