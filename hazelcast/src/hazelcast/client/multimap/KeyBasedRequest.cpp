//
// Created by sancar koyunlu on 9/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/multimap/KeyBasedRequest.h"
#include "hazelcast/client/multimap/MultiMapPortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            KeyBasedRequest::KeyBasedRequest(const std::string& name, const serialization::pimpl::Data& key)
            : name(name)
            , key(key) {

            }

            int KeyBasedRequest::getFactoryId() const {
                return MultiMapPortableHook::F_ID;
            }

            void KeyBasedRequest::write(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", name);
                serialization::ObjectDataOutput& output = writer.getRawDataOutput();
                key.writeData(output);
            }

        }
    }
}


