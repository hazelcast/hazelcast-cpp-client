//
// Created by sancar koyunlu on 9/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/multimap/EntrySetRequest.h"
#include "MultiMapPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            EntrySetRequest::EntrySetRequest(const std::string& name)
            : AllPartitionsRequest(name) {

            };

            int EntrySetRequest::getClassId() const {
                return MultiMapPortableHook::ENTRY_SET;
            };

        }
    }
}