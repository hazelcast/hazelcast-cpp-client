//
// Created by sancar koyunlu on 9/10/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/multimap/AllPartitionsRequest.h"
#include "MultiMapPortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            AllPartitionsRequest::AllPartitionsRequest(const std::string& name)
            : name(name) {

            }

            int AllPartitionsRequest::getFactoryId() const {
                return MultiMapPortableHook::F_ID;
            }

            void AllPartitionsRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", name);
            }

        }
    }
}
