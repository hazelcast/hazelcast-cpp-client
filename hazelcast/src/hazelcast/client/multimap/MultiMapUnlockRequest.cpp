//
// Created by sancar koyunlu on 9/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/multimap/MultiMapUnlockRequest.h"
#include "hazelcast/client/multimap/MultiMapPortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/pimpl/Data.h"


namespace hazelcast {
    namespace client {
        namespace multimap {
            MultiMapUnlockRequest::MultiMapUnlockRequest(const std::string &name, const serialization::pimpl::Data &key, long threadId)
            :KeyBasedRequest(name, key)
            , threadId(threadId)
            , force(false) {

            }

            MultiMapUnlockRequest::MultiMapUnlockRequest(const std::string &name, const serialization::pimpl::Data &key, long threadId, bool force)
            : KeyBasedRequest(name, key)
            , threadId(threadId)
            , force(force) {

            }

            int MultiMapUnlockRequest::getFactoryId() const {
                return MultiMapPortableHook::F_ID;
            }

            int MultiMapUnlockRequest::getClassId() const {
                return MultiMapPortableHook::UNLOCK;
            }


            void MultiMapUnlockRequest::write(serialization::PortableWriter &writer) const {
                writer.writeLong("tid", threadId);
                writer.writeBoolean("force", force);
                KeyBasedRequest::write(writer);
            }


        }
    }
}

