//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/map/ValuesRequest.h"
#include "PortableHook.h"
#include "PortableWriter.h"
#include "PortableReader.h"

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

            void ValuesRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("name", name);
            };

            void ValuesRequest::readPortable(serialization::PortableReader& reader) {
                name = reader.readUTF("name");
            };
        }
    }
}
