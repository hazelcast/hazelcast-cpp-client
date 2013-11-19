//
// Created by sancar koyunlu on 9/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "RemoveAllRequest.h"
#include "MultiMapPortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"


namespace hazelcast {
    namespace client {
        namespace multimap {
            RemoveAllRequest::RemoveAllRequest(const std::string& name, const serialization::Data& key, int threadId)
            : KeyBasedRequest(name, key)
            , threadId(threadId) {

            };

            int RemoveAllRequest::getClassId() const {
                return MultiMapPortableHook::REMOVE_ALL;
            };

            void RemoveAllRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeInt("t", threadId);
                KeyBasedRequest::writePortable(writer);
            };
        }
    }
}

