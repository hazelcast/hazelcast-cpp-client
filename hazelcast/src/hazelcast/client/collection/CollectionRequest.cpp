//
// Created by sancar koyunlu on 9/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/collection/CollectionRequest.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/collection/CollectionPortableHook.h"

namespace hazelcast {
    namespace client {
        namespace collection {

            CollectionRequest::CollectionRequest(const std::string &name, const std::string &serviceName)
            :name(name)
            , serviceName(serviceName) {

            };

            int CollectionRequest::getFactoryId() const {
                return CollectionPortableHook::F_ID;
            };

            void CollectionRequest::write(serialization::PortableWriter &writer) const {
                writer.writeUTF("s", serviceName);
                writer.writeUTF("n", name);
            };

        };
    }
}


