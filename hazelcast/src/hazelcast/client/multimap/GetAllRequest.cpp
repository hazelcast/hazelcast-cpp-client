//
// Created by sancar koyunlu on 9/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/multimap/GetAllRequest.h"
#include "hazelcast/client/multimap/MultiMapPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            GetAllRequest::GetAllRequest(const std::string& name, const serialization::Data& key)
            : KeyBasedRequest(name, key) {

            };

            int GetAllRequest::getClassId() const {
                return MultiMapPortableHook::GET_ALL;
            };
        }
    }
}
