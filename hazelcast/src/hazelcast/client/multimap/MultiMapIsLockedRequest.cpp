//
// Created by sancar koyunlu on 9/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "MultiMapIsLockedRequest.h"
#include "MultiMapPortableHook.h"
#include "PortableWriter.h"


namespace hazelcast {
    namespace client {
        namespace multimap {
            MultiMapIsLockedRequest::MultiMapIsLockedRequest(const std::string& name, const serialization::Data& key, int threadId)
            :KeyBasedRequest(name, key)
            , threadId(threadId) {

            };

            int MultiMapIsLockedRequest::getFactoryId() const {
                return MultiMapPortableHook::F_ID;
            };

            int MultiMapIsLockedRequest::getClassId() const {
                return MultiMapPortableHook::IS_LOCKED;
            };


            void MultiMapIsLockedRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeInt("tid", threadId);
                KeyBasedRequest::writePortable(writer);
            };
        };

    }
}
