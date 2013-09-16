//
// Created by sancar koyunlu on 9/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "CollectionAddListenerRequest.h"
#include "PortableWriter.h"
#include "CollectionPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            CollectionAddListenerRequest::CollectionAddListenerRequest(const std::string& name, bool includeValue)
            :CollectionRequest(name)
            , includeValue(includeValue) {

            };

            int CollectionAddListenerRequest::getClassId() const {
                return CollectionPortableHook::COLLECTION_ADD_LISTENER;
            };

            void CollectionAddListenerRequest::writePortable(serialization::PortableWriter& writer) const {
                CollectionRequest::writePortable(writer);
                writer.writeBoolean("i", includeValue);
            };
        }
    }
}

