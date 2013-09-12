//
// Created by sancar koyunlu on 9/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "CollectionDestroyRequest.h"
#include "CollectionPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            CollectionDestroyRequest::CollectionDestroyRequest(const std::string& name)
            : CollectionRequest(name) {

            }

            int CollectionDestroyRequest::getClassId() const {
                return CollectionPortableHook::COLLECTION_DESTROY;
            }


        }
    }
}