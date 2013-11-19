//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/map/KeySetRequest.h"
#include "hazelcast/client/map/PortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"


namespace hazelcast {
    namespace client {
        namespace map {
            KeySetRequest::KeySetRequest(const std::string& name)
            :name(name) {

            };

            int KeySetRequest::getFactoryId() const {
                return PortableHook::F_ID;
            }

            int KeySetRequest::getClassId() const {
                return PortableHook::KEY_SET;
            }

            void KeySetRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", name);
            };
        }
    }
}

