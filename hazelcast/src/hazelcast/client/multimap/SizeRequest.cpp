//
// Created by sancar koyunlu on 9/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/multimap/SizeRequest.h"
#include "hazelcast/client/multimap/MultiMapPortableHook.h"


namespace hazelcast {
    namespace client {
        namespace multimap {
            SizeRequest::SizeRequest(const std::string& name)
            : AllPartitionsRequest(name) {

            };

            int SizeRequest::getClassId() const {
                return MultiMapPortableHook::SIZE;
            };

            bool SizeRequest::isRetryable() const{
                return true;
            }
        }
    }
}