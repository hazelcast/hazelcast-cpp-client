//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/map/EntrySetRequest.h"
#include "Data.h"
#include "PortableHook.h"
#include "PortableWriter.h"
#include "PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace map {
            EntrySetRequest::EntrySetRequest(const std::string& name)
            :name(name) {

            };

            int EntrySetRequest::getFactoryId() const {
                return PortableHook::F_ID;
            }

            int EntrySetRequest::getClassId() const {
                return PortableHook::ENTRY_SET;
            }

            void EntrySetRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", name);
            };

            void EntrySetRequest::readPortable(serialization::PortableReader& reader) {
                name = reader.readUTF("n");
            };
        }
    }
}
