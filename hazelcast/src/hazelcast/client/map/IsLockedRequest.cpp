//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/map/IsLockedRequest.h"
#include "hazelcast/client/map/PortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace map {
            IsLockedRequest::IsLockedRequest(const std::string &name, serialization::pimpl::Data &key)
            :name(name)
            , key(key){

            };

            int IsLockedRequest::getFactoryId() const {
                return PortableHook::F_ID;
            }

            int IsLockedRequest::getClassId() const {
                return PortableHook::IS_LOCKED;
            }

            void IsLockedRequest::write(serialization::PortableWriter &writer) const {
                writer.writeUTF("n", name);
                writer.writeLong("tid", 0);
                serialization::ObjectDataOutput &out = writer.getRawDataOutput();
                key.writeData(out);
            };

        }
    }
}

