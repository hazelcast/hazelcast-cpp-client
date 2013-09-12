//
// Created by sancar koyunlu on 9/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/multimap/ValuesRequest.h"
#include "MultiMapPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            ValuesRequest::ValuesRequest(const std::string& name)
            : AllPartitionsRequest(name) {

            };

            int ValuesRequest::getClassId() const {
                return MultiMapPortableHook::VALUES;
            };
        }
    }
}


