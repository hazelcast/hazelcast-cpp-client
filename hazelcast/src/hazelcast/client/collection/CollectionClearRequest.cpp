//
// Created by sancar koyunlu on 9/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "CollectionClearRequest.h"
#include "CollectionPortableHook.h"


namespace hazelcast {
    namespace client {
        namespace collection {

            CollectionClearRequest::CollectionClearRequest(const std::string& name)
            : CollectionRequest(name) {

            }

            int CollectionClearRequest::getClassId() const {
                return CollectionPortableHook::COLLECTION_CLEAR;
            }
        }
    }
}