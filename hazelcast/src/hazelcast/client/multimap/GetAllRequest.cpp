//
// Created by sancar koyunlu on 9/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/multimap/GetAllRequest.h"
#include "hazelcast/client/multimap/MultiMapPortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            GetAllRequest::GetAllRequest(const std::string& name, const serialization::pimpl::Data& key, long threadId)
            : KeyBasedRequest(name, key)
            , threadId(threadId) {

            }

            int GetAllRequest::getClassId() const {
                return MultiMapPortableHook::GET_ALL;
            }

            bool GetAllRequest::isRetryable() const {
                return true;
            }

            void GetAllRequest::write(serialization::PortableWriter& writer) const {
                writer.writeLong("threadId", threadId);
                KeyBasedRequest::write(writer);
            }
        }
    }
}

