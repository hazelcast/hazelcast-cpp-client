//
// Created by sancar koyunlu on 9/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/collection/CollectionGetAllRequest.h"
#include "hazelcast/client/collection/CollectionPortableHook.h"


namespace hazelcast {
    namespace client {
        namespace collection {
            CollectionGetAllRequest::CollectionGetAllRequest(const std::string &name, const std::string &serviceName)
            : CollectionRequest(name, serviceName) {

            }

            int CollectionGetAllRequest::getClassId() const {
                return CollectionPortableHook::COLLECTION_GET_ALL;
            }
        }
    }
}