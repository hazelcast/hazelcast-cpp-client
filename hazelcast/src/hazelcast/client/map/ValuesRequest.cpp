//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/map/ValuesRequest.h"
#include "hazelcast/client/map/PortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace map {
            ValuesRequest::ValuesRequest(const std::string& name)
            :name(name) {

            };

            int ValuesRequest::getFactoryId() const {
                return PortableHook::F_ID;
            }

            int ValuesRequest::getClassId() const {
                return PortableHook::VALUES;
            }

            void ValuesRequest::write(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", name);
            };
        }
    }
}
