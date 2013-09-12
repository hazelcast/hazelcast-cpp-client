//
// Created by sancar koyunlu on 9/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "MultiMapDestroyRequest.h"
#include "MultiMapPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            MultiMapDestroyRequest::MultiMapDestroyRequest(const std::string& name)
            : AllPartitionsRequest(name) {

            };

            int MultiMapDestroyRequest::getClassId() const {
                return MultiMapPortableHook::DESTROY;
            };
        }
    }
}
