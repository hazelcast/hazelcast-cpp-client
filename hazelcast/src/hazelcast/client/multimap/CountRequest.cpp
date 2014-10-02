//
// Created by sancar koyunlu on 9/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/multimap/CountRequest.h"
#include "hazelcast/client/multimap/MultiMapPortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            CountRequest::CountRequest(const std::string& name, const serialization::pimpl::Data& key, long threadId)
            : KeyBasedRequest(name, key)
            , threadId(threadId) {

            }

            int CountRequest::getClassId() const {
                return MultiMapPortableHook::COUNT;
            }


            bool CountRequest::isRetryable() const {
                return true;
            }


            void CountRequest::write(serialization::PortableWriter& writer) const {
                writer.writeLong("threadId", threadId);
                KeyBasedRequest::write(writer);

            }
        }
    }
}


