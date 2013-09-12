//
// Created by sancar koyunlu on 9/10/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "AllPartitionsRequest.h"
#include "MultiMapPortableHook.h"
#include "PortableWriter.h"

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
