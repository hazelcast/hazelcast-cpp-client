//
// Created by sancar koyunlu on 9/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/multimap/KeySetRequest.h"
#include "hazelcast/client/multimap/MultiMapPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            KeySetRequest::KeySetRequest(const std::string& name)
            : AllPartitionsRequest(name) {

            };

            int KeySetRequest::getClassId() const {
                return MultiMapPortableHook::KEY_SET;
            };

            bool KeySetRequest::isRetryable() const{
                return true;
            }
        }
    }
}

