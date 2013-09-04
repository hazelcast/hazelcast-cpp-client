//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/map/SizeRequest.h"
#include "PortableHook.h"
#include "PortableWriter.h"
#include "PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace map {
            SizeRequest::SizeRequest(const std::string& name)
            :name(name) {

            };

            int SizeRequest::getFactoryId() const {
                return PortableHook::F_ID;
            }

            int SizeRequest::getClassId() const {
                return PortableHook::SIZE;
            }

            void SizeRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", name);
            };

            void SizeRequest::readPortable(serialization::PortableReader& reader) {
                name = reader.readUTF("n");
            };
        }
    }
}


