//
// Created by sancar koyunlu on 9/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/multimap/RemoveAllRequest.h"
#include "hazelcast/client/multimap/MultiMapPortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"


namespace hazelcast {
    namespace client {
        namespace multimap {
            RemoveAllRequest::RemoveAllRequest(const std::string &name, const serialization::pimpl::Data &key, long threadId)
            : KeyBasedRequest(name, key)
            , threadId(threadId) {

            }

            int RemoveAllRequest::getClassId() const {
                return MultiMapPortableHook::REMOVE_ALL;
            }

            void RemoveAllRequest::write(serialization::PortableWriter &writer) const {
                writer.writeLong("t", threadId);
                KeyBasedRequest::write(writer);
            }
        }
    }
}


