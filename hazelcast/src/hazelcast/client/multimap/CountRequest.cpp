//
// Created by sancar koyunlu on 9/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/multimap/CountRequest.h"
#include "hazelcast/client/multimap/MultiMapPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            CountRequest::CountRequest(const std::string& name, const serialization::pimpl::Data& key)
            :KeyBasedRequest(name, key) {

            };

            int CountRequest::getClassId() const {
                return MultiMapPortableHook::COUNT;
            };


            bool CountRequest::isRetryable() const {
                return true;
            }
        }
    }
}


