//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/map/IsLockedRequest.h"
#include "hazelcast/client/map/PortableHook.h"
#include "hazelcast/client/serialization/Data.h"

#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace map {
            IsLockedRequest::IsLockedRequest(const std::string& name, serialization::Data& key)
            :key(key),
            name(name) {

            };

            int IsLockedRequest::getFactoryId() const {
                return PortableHook::F_ID;
            }

            int IsLockedRequest::getClassId() const {
                return PortableHook::IS_LOCKED;
            }

            void IsLockedRequest::write(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", name);
                writer.writeInt("tid", -1);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                key.writeData(out);
            };

        }
    }
}

