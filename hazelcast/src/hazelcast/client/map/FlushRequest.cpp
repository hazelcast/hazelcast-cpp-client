//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/map/FlushRequest.h"
#include "hazelcast/client/map/PortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"


namespace hazelcast {
    namespace client {
        namespace map {
            FlushRequest::FlushRequest(const std::string& name)
            :name(name) {

            };

            int FlushRequest::getFactoryId() const {
                return PortableHook::F_ID;
            }

            int FlushRequest::getClassId() const {
                return PortableHook::FLUSH;
            }

            void FlushRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("name", name);
            };
        }
    }
}
