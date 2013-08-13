//
// Created by sancar koyunlu on 8/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "AtomicLongRequest.h"
#include "PortableWriter.h"
#include "PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace atomiclong {
            AtomicLongRequest::AtomicLongRequest(const std::string& instanceName, long delta)
            :instanceName(instanceName)
            , delta(delta) {

            };

            int AtomicLongRequest::getFactoryId() const {
                return AtomicLongPortableHook::F_ID;
            };

            void AtomicLongRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", instanceName);
                writer.writeLong("d", delta);
            };

            void AtomicLongRequest::readPortable(serialization::PortableReader& reader) {
                instanceName = reader.readUTF("n");
                delta = reader.readLong("d");
            };

        }
    }
}