//
// Created by sancar koyunlu on 9/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "CollectionGetAllRequest.h"
#include "CollectionPortableHook.h"


namespace hazelcast {
    namespace client {
        namespace collection {
            CollectionGetAllRequest::CollectionGetAllRequest(const std::string& name)
            : CollectionRequest(name) {

            }

            int CollectionGetAllRequest::getClassId() const {
                return CollectionPortableHook::COLLECTION_GET_ALL;
            }
        }
    }
}