//
// Created by sancar koyunlu on 9/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "CountRequest.h"
#include "MultiMapPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            CountRequest::CountRequest(const std::string& name, const serialization::Data& key)
            :KeyBasedRequest(name, key) {

            };

            int CountRequest::getClassId() const {
                return MultiMapPortableHook::COUNT;
            };

        }
    }
}

