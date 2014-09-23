//
// Created by sancar koyunlu on 9/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/multimap/MultiMapIsLockedRequest.h"
#include "hazelcast/client/multimap/MultiMapPortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"


namespace hazelcast {
    namespace client {
        namespace multimap {
            MultiMapIsLockedRequest::MultiMapIsLockedRequest(const std::string &name, const serialization::pimpl::Data &key)
            :KeyBasedRequest(name, key) {

            }

            int MultiMapIsLockedRequest::getFactoryId() const {
                return MultiMapPortableHook::F_ID;
            }

            int MultiMapIsLockedRequest::getClassId() const {
                return MultiMapPortableHook::IS_LOCKED;
            }


            void MultiMapIsLockedRequest::write(serialization::PortableWriter &writer) const {
                writer.writeLong("tid", 0);
                KeyBasedRequest::write(writer);
            }


            bool MultiMapIsLockedRequest::isRetryable() const {
                return true;
            }
        }

    }
}

